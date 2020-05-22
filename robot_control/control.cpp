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
float target_angular_vel = 0.0;
extern char evdevName[50];

int x_axis_max = 32767; //default for smart controller  TG2-850M
int y_axis_max = 32767;

int control_device = 3;
// taken out of main

float checkLinearLimitVelocity(float);
float checkAngularLimitVelocity(float);
std::shared_ptr<rclcpp::Node> node;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// from mqtt_cb_client.c
extern "C"{
    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
    #include "mosquitto.h"
    #include <jansson.h>
}
/*
{"action": "right", "value": "10"}
or
{"action": "right", "value": 10}
 */

#define ACTION_TAG "action"
#define VALUE_TAG "value"
#define QUIT_TAG "quit"
#define ANGULAR_LEFT "left"
#define ANGULAR_RIGHT "right"
#define LINEAR_UP "up"
#define LINEAR_DOWN "down"

static struct mosquitto *mosq = NULL;

int on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg){
	json_t *root;
	json_t *val;
	const char *key;
	json_error_t jerr;
	char *str = (char *)msg->payload;
	char *action = NULL;
	char *value = NULL;
	int num_val = 0;
    static auto control_pub = node->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);
    geometry_msgs::msg::Twist mesg;

	if (str == NULL)
		return -1;

#ifdef DEBUG
	printf("%s %s (%d)\n", msg->topic, (const char *)msg->payload, msg->payloadlen);
#endif

	root = json_loads((char *)msg->payload, 0, &jerr);
	if (NULL == root) {
        printf("%s:%d - error: on line %d: %s\n", __FUNCTION__, __LINE__, jerr.line, jerr.text);
        return 0;
    }

    json_object_foreach(root, key, val) {
		if (strncmp(key, QUIT_TAG, strlen(QUIT_TAG)) == 0) {
			printf("%s:%d -> quit\n", __FUNCTION__, __LINE__);
			mosquitto_disconnect(mosq);
			mosquitto_destroy(mosq);
			mosquitto_lib_cleanup();
			exit(0);
		}
		if (strncmp(key, ACTION_TAG, strlen(ACTION_TAG)) == 0) {
			if (json_is_string(val)) {
				action = strdup(json_string_value(val));
			}
		} 
        else if (strncmp(key, VALUE_TAG, strlen(VALUE_TAG)) == 0) {
			if (json_is_string(val)) {
				value = strdup(json_string_value(val));
				if (value) {
					num_val = atoi(value);
				}
			} 
            else if (json_is_integer(val)) {
				num_val = json_integer_value(val);
			}
		}
	}
    int motionType = 0;
    if (strncmp(action, ANGULAR_LEFT, strlen(ANGULAR_LEFT)) == 0){
        motionType = 1;
    }
    else if (strncmp(action, ANGULAR_RIGHT, strlen(ANGULAR_RIGHT)) == 0){
        motionType = 2;
    }
    else if (strncmp(action, LINEAR_UP, strlen(LINEAR_UP)) == 0){
        motionType = 3;
    }
    else if (strncmp(action, LINEAR_DOWN, strlen(LINEAR_DOWN)) == 0){
        motionType = 4;
    } 
    switch (motionType){
        // angular motion
        case 1:
        // left
            target_angular_vel = target_angular_vel + num_val;
            target_angular_vel = checkAngularLimitVelocity(target_angular_vel);
            // printf("target_angular_vel= %f \n",target_angular_vel);
            break;
        case 2:
        // right
            target_angular_vel = target_angular_vel - num_val;
            target_angular_vel = checkAngularLimitVelocity(target_angular_vel);
            // printf("target_angular_vel= %f \n",target_angular_vel);
            break;
        // linear motion
        case 3:
        // up
            target_linear_vel = target_linear_vel + num_val;
            target_linear_vel = checkLinearLimitVelocity(target_linear_vel);
            break;
        case 4:
        // down
            target_linear_vel = target_linear_vel - num_val;
            target_linear_vel = checkLinearLimitVelocity(target_linear_vel);
            break;
        default:
            break;
    }
    mesg.linear.x = target_linear_vel;
    mesg.angular.z = target_angular_vel;
    // printf("linear x = %f and angular z= %f \n",mesg.linear.x,mesg.angular.z);
    control_pub->publish(mesg);
    motionType = 0;
	// printf("%s:%d - action = %s num_val = %d\n", __FILE__, __LINE__, action, num_val);

	if (action)
		free(action);
	if (value)
		free(value);
	return 0;
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double get_time_now(){
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    return now.tv_sec + now.tv_nsec*1e-9;
}

float checkLinearLimitVelocity(float vel){

    if (vel > BURGER_MAX_LIN_VEL)
        vel = BURGER_MAX_LIN_VEL;

    if (vel <  BURGER_MAX_LIN_VEL_NEG)
        vel = BURGER_MAX_LIN_VEL_NEG;

    return vel;
}

float checkAngularLimitVelocity(float vel){
    if (vel < BURGER_MAX_ANG_VEL_NEG)
        vel = BURGER_MAX_ANG_VEL_NEG;

    if (vel > BURGER_MAX_ANG_VEL)
        vel = BURGER_MAX_ANG_VEL;

    return vel;
}

int main(int argc, char **argv){
    int key = -1;
    int last_key = 0;
    int axis_x = 0;
    int axis_y = 0;
    
    double time_now = get_time_now();
    double loop_refresh = 0.3;
    
    // float rand_turn = 0;
    float demo_fw   = 0.03;
    float demo_ang  = 0.1;

    if (argc == 8 || argc == 10){
        P = atof(argv[1]);
        if (P > 5 || P < -5){
            P = 0.025;
        }           
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
    node = rclcpp::Node::make_shared("control_node");
    auto control_pub_man = node->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);
    float turn = 0;

    rclcpp::WallRate loop_rate(100);
    geometry_msgs::msg::Twist msg;
    int manual_mode = 0;
    // float batt_consume = 0;

    float prev_lin_vel = 0;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // connection from mqtt_cb_client.c
    int rc;

	mosquitto_lib_init();
	
	mosq = mosquitto_new(NULL, true, NULL);
	if (!mosq) {
		fprintf(stderr, "Error: Out of memory.\n");
		return 1;
	}

	char const *ipaddr = "127.0.0.1";
	
	rc = mosquitto_subscribe_callback(
			on_message, NULL,
			"/bb", 0,
			ipaddr, 1883,
			NULL, 60, true,
			NULL, NULL,
			NULL, NULL);

	if (rc) {
		printf("Error: %s\n", mosquitto_strerror(rc));
	}

	
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


    while (rclcpp::ok()){
        rclcpp::spin_some(node);
        mosquitto_loop(mosq, -1, 1);
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
            // removed battery code -sensor reliant

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

            // removed sonar reading
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

        control_pub_man->publish(msg);

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
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // cleanup mqtt_cb_client.c
    mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    return 0;
}

//g++ control.cpp -o control -I/opt/ros/dashing/include/ -L/opt/ros/dashing/lib -lrclcpp -lrcutils -lrcl -lgeometry_msgs__rosidl_typesupport_cpp -lsensor_msgs__rosidl_typesupport_cpp