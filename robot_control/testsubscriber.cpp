#include "rclcpp/rclcpp.hpp"
#include <geometry_msgs/msg/twist.hpp>

rclcpp::Node::SharedPtr g_node = nullptr;

void topic_callback(const geometry_msgs::msg::Twist::SharedPtr msg){
    RCLCPP_INFO(g_node->get_logger(), "I received: linear '%d' and angular '%d'\n", msg->linear.x, msg->angular.z);
}

int main(int argc, char * argv[]){
    rclcpp::init(argc, argv);
    g_node = rclcpp::Node::make_shared("control_node");
    auto subscription =
        g_node->create_subscription<geometry_msgs::msg::Twist>("cmd_vel", 10, topic_callback);
    rclcpp::spin(g_node);
    rclcpp::shutdown();

    subscription = nullptr;
    g_node = nullptr;
    return 0;
}