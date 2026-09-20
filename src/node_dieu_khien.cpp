#include <rclcpp/rclcpp.hpp>
#include <moveit/move_group_interface/move_group_interface.h>
#include <geometry_msgs/msg/pose.hpp>
#include <visualization_msgs/msg/marker.hpp>
#include <cmath>
#include <vector>
#include <map>
#include <thread>
#include <atomic>

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  
  // Bật use_sim_time để khớp 100% với đồng hồ Gazebo
  rclcpp::NodeOptions node_options;
  node_options.automatically_declare_parameters_from_overrides(true);
  node_options.parameter_overrides({{"use_sim_time", true}});
  
  auto node = std::make_shared<rclcpp::Node>("node_dieu_khien", node_options);

  auto marker_pub = node->create_publisher<visualization_msgs::msg::Marker>("/duong_di_viet_chu", 10);

  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node);
  std::thread spinner([&executor]() { executor.spin(); });

  using moveit::planning_interface::MoveGroupInterface;
  MoveGroupInterface move_group(node, "ur_manipulator");

  move_group.setMaxVelocityScalingFactor(0.08);     
  move_group.setMaxAccelerationScalingFactor(0.05); 

  RCLCPP_INFO(node->get_logger(), "=== DANG DONG BO DONG HO VOI GAZEBO ===");

  // 1. TƯ THẾ READY AN TOÀN TRÊN CAO (KHUỶU TAY DỰNG ĐỨNG)
  std::map<std::string, double> tu_the_ready = {
    {"shoulder_pan_joint", 0.0},
    {"shoulder_lift_joint", -1.57},
    {"elbow_joint", 1.57},
    {"wrist_1_joint", -1.57},
    {"wrist_2_joint", -1.57},
    {"wrist_3_joint", 0.0}
  };

  move_group.setJointValueTarget(tu_the_ready);
  MoveGroupInterface::Plan ke_hoach_ready;
  if (move_group.plan(ke_hoach_ready) == moveit::core::MoveItErrorCode::SUCCESS) {
    RCLCPP_INFO(node->get_logger(), "Dang dua robot ve the Ready an toan...");
    move_group.execute(ke_hoach_ready);
  }

  rclcpp::sleep_for(std::chrono::seconds(1));

  // Lấy vị trí thực tế sau khi đồng bộ
  geometry_msgs::msg::Pose pose_ready = move_group.getCurrentPose().pose;
  geometry_msgs::msg::Quaternion huong_chuan = pose_ready.orientation;

  // 2. THIẾT KẾ QUỸ ĐẠO HÌNH TRÒN TRÊN MẶT PHẲNG ĐỨNG (Y-Z)
  const double X_BANG = 0.30;
  const double Y_TAM = 0.0;
  const double Z_TAM = 0.36;
  const double BAN_KINH = 0.07; // Bán kính 7cm (Đường kính 14cm)

  std::vector<geometry_msgs::msg::Pose> danh_sach_hinh_tron;
  geometry_msgs::msg::Pose p;
  p.orientation = huong_chuan;
  p.position.x = X_BANG;

  for (double theta = 0.0; theta <= 2.0 * M_PI + 0.05; theta += 0.1)
  {
    p.position.y = Y_TAM + BAN_KINH * std::cos(theta);
    p.position.z = Z_TAM + BAN_KINH * std::sin(theta);
    danh_sach_hinh_tron.push_back(p);
  }

  // 3. PHÁT MARKER VÒNG TRÒN ĐỎ TRÊN RVIZ LIÊN TỤC
  visualization_msgs::msg::Marker marker;
  marker.header.frame_id = "world";
  marker.ns = "hinh_tron";
  marker.id = 0;
  marker.type = visualization_msgs::msg::Marker::LINE_STRIP;
  marker.action = visualization_msgs::msg::Marker::ADD;
  marker.scale.x = 0.012;
  marker.color.r = 1.0; marker.color.g = 0.0; marker.color.b = 0.0; marker.color.a = 1.0;

  for (const auto& pt : danh_sach_hinh_tron) {
    marker.points.push_back(pt.position);
  }

  std::atomic<bool> dang_chay{true};
  std::thread luong_marker([&]() {
    while (dang_chay && rclcpp::ok()) {
      marker.header.stamp = node->now();
      marker_pub->publish(marker);
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
  });

  // 4. LẬP KẾ HOẠCH CARTESIAN NỐI TỪ VỊ TRÍ HIỆN TẠI VÀO VÒNG TRÒN
  std::vector<geometry_msgs::msg::Pose> toan_bo_quy_dao;
  toan_bo_quy_dao.push_back(danh_sach_hinh_tron.front()); // Lướt 2cm vào mép vòng tròn
  for (const auto& pt : danh_sach_hinh_tron) {
    toan_bo_quy_dao.push_back(pt);                        // Quay trọn vẹn 1 vòng tròn
  }

  moveit_msgs::msg::RobotTrajectory quy_dao_tron;
  double ti_le = move_group.computeCartesianPath(toan_bo_quy_dao, 0.008, 0.0, quy_dao_tron);
  RCLCPP_INFO(node->get_logger(), "Ti le lap ke hoach hinh tron: %.2f%%", ti_le * 100.0);

  if (ti_le > 0.85) {
    RCLCPP_INFO(node->get_logger(), " BAT DAU VE HINH TRON");
    move_group.execute(quy_dao_tron);
    RCLCPP_INFO(node->get_logger(), "HOAN THANH VE HINH TRON");
  } else {
    RCLCPP_ERROR(node->get_logger(), "Ti le quy dao chua dat!");
  }

  rclcpp::sleep_for(std::chrono::seconds(180));

  dang_chay = false;
  if (luong_marker.joinable()) luong_marker.join();
  spinner.join();
  rclcpp::shutdown();
  return 0;
}