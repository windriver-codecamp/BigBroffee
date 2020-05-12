#include <geometry_msgs/msg/twist.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <turtlebot3_msgs/msg/sensor_state.hpp>

#ifdef __VXWORKS__
#include "keyboard_event.hpp"
#include "gamepad_event.hpp"
#endif
using std::placeholders::_1;

float BURGER_MAX_LIN_VEL = 0.22;
float BURGER_MAX_LIN_VEL_NEG = -0.22;
float BURGER_MAX_ANG_VEL = 1.84;
float BURGER_MAX_ANG_VEL_NEG = -1.84;

float LIN_VEL_STEP_SIZE = 0.01;
float ANG_VEL_STEP_SIZE = 0.1;

float P = 0.025;

float lidar_min_range = 0.5;
float sonar_min_range = 5;

float sum_anglesL = 0;
float sum_anglesR = 0;

float target_linear_vel = 0.0;
extern char evdevName[50];

int x_axis_max = 32767; //default for smart controller  TG2-850M
int y_axis_max = 32767; //

int control_device = 3;
/*Lidar Subscriber*/

sensor_msgs::msg::LaserScan::SharedPtr lidar_msg;
int lidar_read = 0;
// class LidarSubscriber : public rclcpp::Node
// {
// public:
//     LidarSubscriber()
//         : Node("Lidar_subscriber")
//     {
//         auto qos = rclcpp::QoS(rclcpp::SensorDataQoS());
//         subscription_ = this->create_subscription<sensor_msgs::msg::LaserScan>("scan",
//                                                                                qos,
//                                                                                std::bind(&LidarSubscriber::lidar_callback, 
//                                                                                this,
//                                                                                std::placeholders::_1));
//     }

//     void lidar_callback(sensor_msgs::msg::LaserScan::SharedPtr msg)
//     {
//         int i = 0;
//         lidar_msg = msg;
//         /*for (i = 0; i < 360; i++)
//         {
//             RCLCPP_INFO(this->get_logger(), "I heard: r[%d] = '%f'",i ,lidar_msg->ranges[i]);
//         }*/

//         lidar_read = 1;
//     }

//     rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr subscription_;
// };

/*Sonar Subscriber*/

turtlebot3_msgs::msg::SensorState::SharedPtr sensors_msg;
int sonar_read = 0;
// class SonarSubscriber : public rclcpp::Node
// {
// public:
//     SonarSubscriber()
//         : Node("Sonar_subscriber")
//     {
//         auto qos = rclcpp::QoS(rclcpp::SensorDataQoS());
//         subscription_ = this->create_subscription<turtlebot3_msgs::msg::SensorState>("sensor_state",
//                                                                                      qos, 
//                                                                                      std::bind(&SonarSubscriber::sonar_callback,
//                                                                                      this, 
//                                                                                      std::placeholders::_1));
//     }

//     void sonar_callback(turtlebot3_msgs::msg::SensorState::SharedPtr msg)
//     {
//         int i = 0;
//         sensors_msg = msg;
//         //RCLCPP_INFO(this->get_logger(), "Sonar reading '%f'",i ,sonar_msg->sonar);
//         //RCLCPP_INFO(this->get_logger(), "Sonar reading '%f'",i ,sonar_msg->battery);
//         sonar_read = 1;
//     }
//     rclcpp::Subscription<turtlebot3_msgs::msg::SensorState>::SharedPtr subscription_;
// };

// Subscriber mqtt from minimal subscriber ros2
#include "std_msgs/msg/string.hpp"
rclcpp::Node::SharedPtr mqtt_node = nullptr;

void topic_callback(const std_msgs::msg::String::SharedPtr msg)
{
  RCLCPP_INFO(mqtt_node->get_logger(), "I heard: '%s'", msg->data.c_str());
}
// din codul lui dan trebuie pus aici subscriberul
//trebuie publisher ros
//trebuie făcut folder în repo git pentru chestiile care le fac eu http://bitbucket.wrs.com/users/mdragusu/repos/ball_follower_robot/browse într-un folder aici
// trimit mesaj în mqtt
//trebuie să am grijă cum copiez lib-urile în sdk, am tot ce-mi trebuie în repo, trebuie modificat cmakefile-ul puţin

double get_time_now()
{
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    return now.tv_sec + now.tv_nsec*1e-9;
}


float autoturn()
{

    float med = 0;
    int i = 0;
    sum_anglesR = 0;
    sum_anglesL = 0;
    for (i = 0; i < 90; i++)
    {
        if (lidar_msg->ranges[i] < lidar_min_range && lidar_msg->intensities[i] > 150 &&  lidar_msg->ranges[i] != 0)
        {
            if (target_linear_vel != 0)
                sum_anglesL += (90 - i) * (3.5 - lidar_msg->ranges[i]) * P * (target_linear_vel * 100);
            else
                sum_anglesL += (90 - i) * (3.5 - lidar_msg->ranges[i]) * P;
        }

        if (lidar_msg->ranges[270 + i] < lidar_min_range && lidar_msg->intensities[270 + i] > 150 && lidar_msg->ranges[270+1] != 0)
        {
            if (target_linear_vel != 0)
                sum_anglesR += i * (3.5 - lidar_msg->ranges[270 + i]) * P * (target_linear_vel * 100);
            else
                sum_anglesR += i * (3.5 - lidar_msg->ranges[270 + i]) * P;
        }
    }

    med = ((sum_anglesR - sum_anglesL) / 90) * 0.1;

    return med;
}

float checkLinearLimitVelocity(float vel)
{

    if (vel > BURGER_MAX_LIN_VEL)
        vel = BURGER_MAX_LIN_VEL;

    if (vel <  BURGER_MAX_LIN_VEL_NEG)
        vel = BURGER_MAX_LIN_VEL_NEG;

    return vel;
}

float checkAngularLimitVelocity(float vel)
{
    if (vel < BURGER_MAX_ANG_VEL_NEG)
        vel = BURGER_MAX_ANG_VEL_NEG;

    if (vel > BURGER_MAX_ANG_VEL)
        vel = BURGER_MAX_ANG_VEL;

    return vel;
}

int main(int argc, char **argv)
{
    int key = -1;
    int last_key = 0;
    int axis_x = 0;
    int axis_y = 0;
    
    double time_now = get_time_now();
    double sonar_protocol_time = 0;
    double loop_refresh = 0.3;
    
    int sonar_step  = 0;
    float rand_turn = 0;
    float demo_fw   = 0.03;
    float demo_ang  = 0.1;

    if (argc == 8 || argc == 10)
    {
        P = atof(argv[1]);
        if (P > 5 || P < -5)
        {
            P = 0.025;
        }           
        
        lidar_min_range = atof(argv[2]); // distance in meters for lidar to detect obstacles
        sonar_min_range = atof(argv[3]); // distance  in cm for sonar to trigger emergency action 
        control_device  = atoi(argv[4]); // 1 for keyboard 2 for gamepad  3 for both
        loop_refresh    = atof(argv[5]); // loop printout refresh rate
        demo_fw         = atof(argv[6]); // demo mode forward speed
        demo_ang        = atof(argv[7]); // demo mode angular speed
        if (argc == 10)
        {   
            y_axis_max = atoi(argv[8]); //value got from the calibration app
            x_axis_max = atoi(argv[9]); //value got from the calibration app
        }
    }

    identify_gamepad();

    rclcpp::init(argc, argv);
    auto node = rclcpp::Node::make_shared("control_node");
    // auto node_sub = std::make_shared<LidarSubscriber>();
    // auto sonar_node_sub = std::make_shared<SonarSubscriber>();
    auto control_pub = node->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);
    mqtt_node = rclcpp::Node::make_shared("minimal_subscriber");
  auto subscription =
    mqtt_node->create_subscription<std_msgs::msg::String>("MQTT", 10, topic_callback);

    rclcpp::WallRate loop_rate(100);
    geometry_msgs::msg::Twist msg;
    float turn = 0;
    float target_angular_vel = 0.0;

    int manual_mode = 0;
    float batt_consume = 0;

    float prev_lin_vel = 0;
    while (rclcpp::ok())
    {

        // rclcpp::spin_some(node_sub);
        // rclcpp::spin_some(sonar_node_sub);
        rclcpp::spin_some(node);
        rclcpp::spin(mqtt_node);//added

        /*autonoumous mode*/
        if (lidar_read == 1 && (manual_mode == 0 || manual_mode == 2))
        {
                           
            turn = autoturn();
            target_angular_vel = checkAngularLimitVelocity(turn);

            if (sonar_read == 1)
            {   
                /*emergency behaviour in autonomous mode
                  if sonar limit is hit, turn back around
                */
                if (sensors_msg->sonar < sonar_min_range && sensors_msg->sonar != 0 && sonar_step == 0)
                {
                    sonar_step = 1;
                    prev_lin_vel = target_linear_vel;
                    sonar_protocol_time = get_time_now();
                }
            }    

            switch (sonar_step)
            {
            case 1:
                //Step 1 move backwards
                if (get_time_now() - sonar_protocol_time  < 3)
                {
                    target_linear_vel= -0.02;
                    target_angular_vel = 0;
                }
                else
                {
                    sonar_step = 2;
                    sonar_protocol_time = get_time_now();
                    //generate random turn after wal was reached
                    rand_turn = rand() % 70;
                }
                break;
            case 2:
                //Step 2 Turn around
                if (get_time_now() - sonar_protocol_time < 7.5)
                {
                    target_linear_vel= 0.001;                  
                    if (rand_turn >= 35)
                    {
                        target_angular_vel = rand_turn / 100;
                        if (target_angular_vel < 0.45)
                            target_angular_vel = 0.45;
                    } else
                    {
                        target_angular_vel = - rand_turn / 100;
                        if (target_angular_vel > -0.45)
                            target_angular_vel = -0.45;
                    }                            
                }
                else
                {
                    sonar_step = 3;
                    sonar_protocol_time = get_time_now();
                }
                break;
            case 3:
                //Step 3 Wait a bit 
                if (get_time_now() - sonar_protocol_time < 3)
                {
                    target_linear_vel= 0.001;
                }
                else
                {
                    sonar_step = 0;
                    target_linear_vel = prev_lin_vel;
                    sonar_protocol_time = get_time_now();
                }
                break;
            default:
                break;
            }
            
        }

        if (sonar_read == 1 && manual_mode == 1) /*sonar emergency stop*/
        {
            /*emergency behaviour in manual mode
             if sonar limit is hit, go back a little
            */
            if (sensors_msg->sonar < sonar_min_range && sensors_msg->sonar != 0)
            {
                msg.linear.x = -0.02;
                msg.angular.z = 0;
                control_pub->publish(msg);
                rclcpp::spin_some(node);
                sleep(2);
                target_linear_vel = 0.0;
                target_angular_vel = 0.0;
            }
        }


        /*manual control*/
        switch (key)
        {
        case 119:   //w
        case 57351: //up
        case BTN_NORTH:
            target_linear_vel = target_linear_vel + LIN_VEL_STEP_SIZE;
            target_linear_vel = checkLinearLimitVelocity(target_linear_vel);
            break;
        case 100:   //d
        case 57350: //right
        case BTN_EAST:
            target_angular_vel = target_angular_vel - ANG_VEL_STEP_SIZE;
            target_angular_vel = checkAngularLimitVelocity(target_angular_vel);
            break;
        case 97:    //a
        case 57349: //left
        case BTN_WEST:
            target_angular_vel = target_angular_vel + ANG_VEL_STEP_SIZE;
            target_angular_vel = checkAngularLimitVelocity(target_angular_vel);
            break;
        case 32:    // space
            target_angular_vel = 0.0;
            target_linear_vel = 0.0;     
            break;  
        case 115:   //s
        case 57352: //down
        case BTN_SOUTH:
            target_linear_vel = target_linear_vel - LIN_VEL_STEP_SIZE;
            target_linear_vel = checkLinearLimitVelocity(target_linear_vel);
            break;
        case 112: //p
            manual_mode = 2;
            break;   
        case 111: //o
            manual_mode = 0;
            break;                   
        case 109: //m
            manual_mode = 1;
            target_angular_vel = 0.0;
            target_linear_vel = 0.0;
            break;
        case BTN_TL:
            manual_mode = 2;
            break;
        case BTN_TR:
            if (manual_mode == 0)
            {
                manual_mode = 1;
                target_angular_vel = 0.0;
                target_linear_vel = 0.0;
            }
            else
            {
                manual_mode = 0;
            }
            break;
        case 117: //u
            P += 0.005;
            break;
        case 106: //j
            P -= 0.005;
            break;
        default:
            break;
        }

        if (manual_mode == 1)
        {
            if (axis_y != 0)
            {
                target_linear_vel = checkLinearLimitVelocity((axis_y * BURGER_MAX_LIN_VEL) / (y_axis_max * 2));
            }
            
            if (axis_x != 0)
            {
                target_angular_vel = checkAngularLimitVelocity ((-axis_x *  BURGER_MAX_ANG_VEL) / (x_axis_max * 2));
            }
        }

        /*demo mode*/
        if (manual_mode == 2)
        {
            target_linear_vel = demo_fw;
            target_angular_vel = target_angular_vel + demo_ang;

        }

        if (key != -1 && last_key == 0)
        {
            last_key = key;
        }

        if (key != -1 && last_key != key)
        {
            last_key = key;
        }

        if (get_time_now() - time_now > loop_refresh){     
            printf("\033[2J");
            printf("\033[1;1H");     
            /*print area start */
            if (sonar_read == 1)
            {
                batt_consume = ((12.3 - sensors_msg->battery) * 100.0) / 1.8; //   battery at 12.3V is 100% and at 10.5V is 0%
                printf("Battery at %f % \t V = %f \n", 100.0 - batt_consume, sensors_msg->battery);
            } else
            {
                printf("Battery info NA \n");
            }

            printf("currently:\tlinear vel %f\t angular vel %f \n", target_linear_vel, target_angular_vel);
            printf("turning: %f \n", turn);
            if (manual_mode == 0)
            {
                printf("Autonomous Mode \n");
            }
            else if (manual_mode == 1)
            {
                printf("Manual Mode \n");
            }
            else if (manual_mode == 2)
            {
                printf("Demo Mode: true \n");
            }
            printf("Input %d \n", last_key);

            if (sonar_read == 1)
            {
                printf("Sonar reading at %f \n", sensors_msg->sonar);
            } else {
                printf("No sonar reading \n");
            }
            printf("Controller P: %f \n", P);

            printf("Left side threat: %f \n", sum_anglesL);
            printf("Right side threat: %f \n", sum_anglesR);
            printf ("xaxis %d \n",axis_x);
            printf ("yaxis %d \n",axis_y);
            time_now = get_time_now();
            /*print area end */
        }

        msg.linear.x = target_linear_vel;
        msg.angular.z = target_angular_vel;

        control_pub->publish(msg);

        if (control_device == 1)
        {
            key = WaitKey(50);
        } else if (control_device == 2)
        {
            key = WaitGKey(50, &axis_x, &axis_y);
        } else if (control_device == 3)
        {
            key = WaitKey(50);
            if (key == -1)
               key = WaitGKey(50, &axis_x, &axis_y);
        } else
        {
            printf("Please select a valid control device \n 1 for keyboard \n 2 for gamepad \n 3 for both\n");
            return -1;
        }
        
            
    }
    rclcpp::shutdown();

    return 0;
}

//g++ control.cpp -o control -I/opt/ros/dashing/include/ -L/opt/ros/dashing/lib -lrclcpp -lrcutils -lrcl -lgeometry_msgs__rosidl_typesupport_cpp -lsensor_msgs__rosidl_typesupport_cpp