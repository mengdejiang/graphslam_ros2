#ifndef GS_SM_COMPONENT_H_INCLUDED
#define GS_SM_COMPONENT_H_INCLUDED

#if __cplusplus
extern "C" {
#endif

// The below macros are taken from https://gcc.gnu.org/wiki/Visibility and from
// demos/composition/include/composition/visibility_control.h at https://github.com/ros2/demos
#if defined _WIN32 || defined __CYGWIN__
  #ifdef __GNUC__
    #define GS_SM_EXPORT __attribute__ ((dllexport))
    #define GS_SM_IMPORT __attribute__ ((dllimport))
  #else
    #define GS_SM_EXPORT __declspec(dllexport)
    #define GS_SM_IMPORT __declspec(dllimport)
  #endif
  #ifdef GS_SM_BUILDING_DLL
    #define GS_SM_PUBLIC GS_SM_EXPORT
  #else
    #define GS_SM_PUBLIC GS_SM_IMPORT
  #endif
  #define GS_SM_PUBLIC_TYPE GS_SM_PUBLIC
  #define GS_SM_LOCAL
#else
  #define GS_SM_EXPORT __attribute__ ((visibility("default")))
  #define GS_SM_IMPORT
  #if __GNUC__ >= 4
    #define GS_SM_PUBLIC __attribute__ ((visibility("default")))
    #define GS_SM_LOCAL  __attribute__ ((visibility("hidden")))
  #else
    #define GS_SM_PUBLIC
    #define GS_SM_LOCAL
  #endif
  #define GS_SM_PUBLIC_TYPE
#endif

#if __cplusplus
} // extern "C"
#endif

#include <rclcpp/rclcpp.hpp>

#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>
#include <tf2_ros/transform_broadcaster.h>
#include <tf2_sensor_msgs/tf2_sensor_msgs.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include <tf2_eigen/tf2_eigen.h>

#include <sensor_msgs/msg/point_cloud2.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <geometry_msgs/msg/point.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/transform.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <nav_msgs/msg/path.hpp>

#include <graphslam_ros2_msgs/msg/map_array.hpp>

#include <pclomp/ndt_omp.h>
#include <pclomp/ndt_omp_impl.hpp>
#include <pclomp/voxel_grid_covariance_omp.h>
#include <pclomp/voxel_grid_covariance_omp_impl.hpp>
#include <pclomp/gicp_omp.h>
#include <pclomp/gicp_omp_impl.hpp>

#include <pcl_conversions/pcl_conversions.h>

#include <pcl/point_types.h>

namespace graphslam
{
    class ScanMatcherComponent: public rclcpp::Node
    {
    public:
        GS_SM_PUBLIC
        explicit ScanMatcherComponent(const rclcpp::NodeOptions & options);
    private:
        rclcpp::Clock clock_;
        tf2_ros::Buffer tfbuffer_;
        tf2_ros::TransformListener listener_;
        tf2_ros::TransformBroadcaster broadcaster_;

        std::string global_frame_id_;
        
        pcl::Registration<pcl::PointXYZI, pcl::PointXYZI>::Ptr registration_;
        pcl::VoxelGrid<pcl::PointXYZI> voxelgrid_;

        Eigen::Vector3d previous_position_;
        double trans_;
        double latest_distance_{0};

        double previous_time_odom_{-1};
        rclcpp::Time current_stamp_;
        Eigen::Vector3d rollpitchyaw_{0, 0, 0};

        rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr initial_pose_sub_;
        rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_sub_;
        rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
        rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr input_cloud_sub_;

        geometry_msgs::msg::PoseStamped corrent_pose_stamped_;
        pcl::PointCloud<pcl::PointXYZI> map_;
        graphslam_ros2_msgs::msg::MapArray map_array_msg_;
        rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr pose_pub_;
        rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr map_pub_;
        rclcpp::Publisher<graphslam_ros2_msgs::msg::MapArray>::SharedPtr map_array_pub_;
        rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr path_pub_;

        void initializePubSub();
        void receiveCloud(const pcl::PointCloud<pcl::PointXYZI>::ConstPtr& input_cloud_ptr, rclcpp::Time stamp);
        void receiveImu(const sensor_msgs::msg::Imu imu_msg);
        void receiveOdom(const nav_msgs::msg::Odometry odom_msg);
        void publishMapAndPose(const pcl::PointCloud<pcl::PointXYZI>::ConstPtr& cloud_ptr, Eigen::Matrix4f final_transformation, rclcpp::Time stamp);
        Eigen::Matrix4f getSimTrans(geometry_msgs::msg::PoseStamped pose_stamped);

        bool initial_pose_received_{false};
        bool initial_cloud_received_{false};
        double trans_for_mapupdate_;
        double vg_size_for_input_;
        double vg_size_for_map_;

        bool use_odom_{false};
        bool use_imu_{false};

    };
}

#endif  //GS_SM_COMPONENT_H_INCLUDED
