/*
João Avelino
December, 2018
*/

#include <stdio.h>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <math.h> 
#include "../include/vizzy_rviz_plugins/grasp_panel.hpp"


namespace vizzy_rviz_plugins {

GraspPanel::GraspPanel(QWidget *parent)
  : rviz::Panel(parent), tfBuffer(), tfListener(tfBuffer)
{

  //Configure the push buttons for the handshakes
  go_to_goal = new QPushButton("Go to goal!", this);
  home_button = new QPushButton("Home position", this);
  grab_button = new QPushButton("Grab", this);
  release_button = new QPushButton("Release", this);
  putinbox_button = new QPushButton("Put in box", this);
  giveaway_button = new QPushButton("Give away", this);

  freeze_goal_ = false;
  freeze_goal_button_ = new QPushButton("Freeze goal!", this);


  // Next we lay out the "action" text entry field using a
  // QLabel and a QLineEdit in a QHBoxLayout.
  QHBoxLayout* action_layout = new QHBoxLayout();
  action_layout->addWidget( new QLabel( "Action client:" ));
  output_action_editor_ = new QLineEdit;
  output_action_editor_->setText("/vizzy/left_arm_cartesian_controller/cartesian_action");
  action_layout->addWidget(output_action_editor_ );

  //Initialize action client
  ac = std::make_shared<cartesian_client>(output_action_editor_->text().toStdString(), true);

  // Next we lay out the "topic" text entry field using a
  // QLabel and a QLineEdit in a QHBoxLayout.
  QHBoxLayout* topic_layout = new QHBoxLayout();
  topic_layout->addWidget( new QLabel( "Input goal topic:" ));
  input_topic_editor_ = new QLineEdit;
  input_topic_editor_->setText("/left_hand_goal");
  topic_layout->addWidget(input_topic_editor_ );


  //Initialize goal action to update the goal from other nodes (example: ball tracker)
  goal_sub_ = nh_.subscribe<geometry_msgs::PoseStamped>(input_topic_editor_->text().toStdString(), 1, &GraspPanel::poseCallback, this);
  goal_pub_ = nh_.advertise<geometry_msgs::PoseStamped>("/grasp_goal_vis", 1);

  QVBoxLayout* gestures_layout= new QVBoxLayout();
  gestures_layout->addWidget(go_to_goal);
  gestures_layout->addWidget(home_button);
  gestures_layout->addWidget(grab_button);
  gestures_layout->addWidget(release_button);
  gestures_layout->addWidget(putinbox_button);
  gestures_layout->addWidget(giveaway_button);

  QVBoxLayout* controls_out_layout= new QVBoxLayout();

  QGridLayout *controls_layout = new QGridLayout;
  controlsGroup_ = new QGroupBox("End effector controls (current goal)");
  x_label_ = new QLabel(tr("x (m):"));
  y_label_ = new QLabel(tr("y (m):"));
  z_label_ = new QLabel(tr("z (m):"));

  x_angle_label_ = new QLabel(tr("orientation x (quat):"));
  y_angle_label_ = new QLabel(tr("orientation y (quat):"));
  z_angle_label_ = new QLabel(tr("orientation z (quat):"));
  w_angle_label_ = new QLabel(tr("orientation w (quat):"));


  x_spin_ = new QDoubleSpinBox();
  x_spin_->setRange(-5, 5);
  x_spin_->setSingleStep(0.01);

  y_spin_ = new QDoubleSpinBox();
  y_spin_->setRange(-5, 5);
  y_spin_->setSingleStep(0.01);

  z_spin_ = new QDoubleSpinBox();
  z_spin_->setRange(-5, 5);
  z_spin_->setSingleStep(0.01);


  x_angle_spin_ = new QDoubleSpinBox();
  x_angle_spin_->setRange(-1.0, 1.0);
  x_angle_spin_->setSingleStep(0.01);
  x_angle_spin_->setReadOnly(true);

  y_angle_spin_ = new QDoubleSpinBox();
  y_angle_spin_->setRange(-1.0, 1.0);
  y_angle_spin_->setSingleStep(0.01);
  y_angle_spin_->setReadOnly(true);

  z_angle_spin_ = new QDoubleSpinBox();
  z_angle_spin_->setRange(-1.0, 1.0);
  z_angle_spin_->setSingleStep(0.01);
  z_angle_spin_->setReadOnly(true);

  w_angle_spin_ = new QDoubleSpinBox();
  w_angle_spin_->setRange(-1.0, 1.0);
  w_angle_spin_->setSingleStep(0.01);
  w_angle_spin_->setReadOnly(true);

  controls_layout->addWidget(x_label_, 0, 0);
  controls_layout->addWidget(x_spin_, 0, 1);
  controls_layout->addWidget(y_label_, 1, 0);
  controls_layout->addWidget(y_spin_, 1, 1);
  controls_layout->addWidget(z_label_, 2, 0);
  controls_layout->addWidget(z_spin_, 2, 1);

  controls_layout->addWidget(x_angle_label_, 3, 0);
  controls_layout->addWidget(x_angle_spin_, 3, 1);
  controls_layout->addWidget(y_angle_label_, 4, 0);
  controls_layout->addWidget(y_angle_spin_, 4, 1);
  controls_layout->addWidget(z_angle_label_, 5, 0);
  controls_layout->addWidget(z_angle_spin_, 5, 1);
  controls_layout->addWidget(w_angle_label_, 6, 0);
  controls_layout->addWidget(w_angle_spin_, 6, 1);


  controls_out_layout->addLayout(controls_layout);
  controls_out_layout->addWidget(freeze_goal_button_);
  controlsGroup_->setLayout(controls_out_layout);



  QVBoxLayout* panel_layout = new QVBoxLayout();
  panel_layout->addWidget(controlsGroup_);
  panel_layout->addLayout(gestures_layout);
  panel_layout->addLayout(action_layout);
  panel_layout->addLayout(topic_layout);

  setLayout( panel_layout );

  //Connect objects with signals
  connect(home_button, SIGNAL (released()), this, SLOT(home()));
  connect(grab_button, SIGNAL (released()), this, SLOT (grab()));
  connect(release_button, SIGNAL (released()), this, SLOT (release()));
  connect(putinbox_button, SIGNAL (released()), this, SLOT (putinbox()));
  connect(giveaway_button, SIGNAL (released()), this, SLOT (giveaway()));
  connect(go_to_goal, SIGNAL (released()), this, SLOT (gotoGoal()));
  connect( output_action_editor_, SIGNAL( editingFinished() ), this, SLOT( updateAction() ));
  connect( input_topic_editor_, SIGNAL( editingFinished() ), this, SLOT( updateTopic() ));
  
  connect( x_spin_, SIGNAL( valueChanged(double) ), this, SLOT( updateGoalX() ));
  connect( y_spin_, SIGNAL( valueChanged(double) ), this, SLOT( updateGoalY() ));
  connect( z_spin_, SIGNAL( valueChanged(double) ), this, SLOT( updateGoalZ() ));

  connect(freeze_goal_button_, SIGNAL (released()), this, SLOT (freezeUnfreeze()));


  updateAction();
  updateTopic();


  //Interactive markers
  
  int_marker_.header.frame_id = "base_link";
  int_marker_.scale = 1;
  int_marker_.name = "end_effector_";
  int_marker_.description = "End effector";

  //insert the end effector
  makeEndEffectorControl(int_marker_);
  int_marker_.controls[0].interaction_mode = visualization_msgs::InteractiveMarkerControl::MOVE_ROTATE_3D;


}

void GraspPanel::updateMarkerPose()
{

  
}


void GraspPanel::poseCallback(const geometry_msgs::PoseStamped::ConstPtr& msg)
{
  
  if(freeze_goal_)
    return;

  geometry_msgs::TransformStamped transformStamped;
  geometry_msgs::PoseStamped onBase;

  try{
  transformStamped = tfBuffer.lookupTransform("base_link", msg->header.frame_id,
                              ros::Time(0));
  tf2::doTransform(*msg, onBase, transformStamped);

  }
  catch (tf2::TransformException &ex) {
    ROS_WARN("%s",ex.what());
    ros::Duration(1.0).sleep();
    return;
  }


  goal_pos_x_ = onBase.pose.position.x;
  goal_pos_y_ = onBase.pose.position.y;
  goal_pos_z_ = onBase.pose.position.z;

  goal_orient_x_ = onBase.pose.orientation.x;
  goal_orient_y_ = onBase.pose.orientation.y;
  goal_orient_z_ = onBase.pose.orientation.z;


  x_spin_->setValue(goal_pos_x_);
  y_spin_->setValue(goal_pos_y_);
  z_spin_->setValue(goal_pos_z_);

  geometry_msgs::PoseStamped pose_viz;

  pose_viz.header.frame_id="base_link";

  pose_viz.pose.position.x = goal_pos_x_+goal_pos_x_offset_;
  pose_viz.pose.position.y = goal_pos_y_+goal_pos_y_offset_;
  pose_viz.pose.position.z = goal_pos_z_+goal_pos_z_offset_;

  double o_x = goal_orient_x_+goal_orient_x_offset_;
  double o_y = goal_orient_y_+goal_orient_y_offset_;
  double o_z = goal_orient_z_+goal_orient_z_offset_;

  double w = std::sqrt(1.0-(o_x*o_x+o_y*o_y+o_z*o_z));

  x_angle_spin_->setValue(goal_orient_x_);
  y_angle_spin_->setValue(goal_orient_y_);
  z_angle_spin_->setValue(goal_orient_z_);
  w_angle_spin_->setValue(w);


  pose_viz.pose.orientation.x = o_x;
  pose_viz.pose.orientation.y = o_y;
  pose_viz.pose.orientation.z = o_z; 
  pose_viz.pose.orientation.w = w;

  goal_pub_.publish(pose_viz);

}

void GraspPanel::gotoGoal()
{
  vizzy_msgs::CartesianGoal goal;
  goal.type = goal.CARTESIAN;
  goal.end_effector_pose.pose.position.x = goal_pos_x_+goal_pos_x_offset_;
  goal.end_effector_pose.pose.position.y = goal_pos_y_+goal_pos_y_offset_;
  goal.end_effector_pose.pose.position.z = goal_pos_z_+goal_pos_z_offset_;
  

  double o_x = goal_orient_x_+goal_orient_x_offset_;
  double o_y = goal_orient_y_+goal_orient_y_offset_;
  double o_z = goal_orient_z_+goal_orient_z_offset_;
  goal.end_effector_pose.pose.orientation.x = o_x;
  goal.end_effector_pose.pose.orientation.y = o_y;
  goal.end_effector_pose.pose.orientation.z = o_z; 
  goal.end_effector_pose.pose.orientation.w = std::sqrt(1.0-(o_x*o_x+o_y*o_y+o_z*o_z));

  ac->sendGoal(goal);
}

void GraspPanel::home()
{
  vizzy_msgs::CartesianGoal goal;
  goal.type = goal.HOME;
  ac->sendGoal(goal);
}

void GraspPanel::grab()
{
  vizzy_msgs::CartesianGoal goal;
  goal.type = goal.GRAB;
  ac->sendGoal(goal);
}

void GraspPanel::release()
{
  vizzy_msgs::CartesianGoal goal;
  goal.type = goal.RELEASE;
  ac->sendGoal(goal);
}

void GraspPanel::putinbox()
{


}

void GraspPanel::giveaway()
{
}

void GraspPanel::updateAction()
{
  setAction( output_action_editor_->text() );
}

void GraspPanel::setAction(const QString &new_action)
{
  // Only take action if the name has changed.
  if( new_action != output_action_ )
  {
    output_action_ = new_action;
    // If the action is the empty string, don't publish anything.
    if( output_action_ == "" )
    {
      ac->cancelAllGoals();
      ac = std::make_shared<cartesian_client>("null", true);
    }
    else
    {
      ac->cancelAllGoals();
      ac = std::make_shared<cartesian_client>(output_action_.toStdString(), true);      
    }

    Q_EMIT configChanged();
  }
}

void GraspPanel::updateTopic()
{
  setTopic( input_topic_editor_->text() );
}

void GraspPanel::setTopic(const QString &new_topic)
{
  // Only take topic if the name has changed.
  if( new_topic != input_topic_ )
  {
    input_topic_ = new_topic;
    // If the action is the empty string, don't subscribe anything.
    if( input_topic_ == "" )
    {
      goal_sub_.shutdown();
    }
    else
    {
      goal_sub_ = nh_.subscribe<geometry_msgs::PoseStamped>(input_topic_.toStdString(), 1, &GraspPanel::poseCallback, this);
    }

    Q_EMIT configChanged();
  }
}

void GraspPanel::freezeUnfreeze()
{
  if(freeze_goal_)
  {
    freeze_goal_ = false;
    freeze_goal_button_->setText("Freeze goal!");
  }
  else{
    freeze_goal_ = true;
    freeze_goal_button_->setText("Unfreeze goal!");
  }

}


void GraspPanel::updateGoalX()
{
  goal_pos_x_ = (double) (x_spin_->value());
  geometry_msgs::PoseStamped pose_viz;

  pose_viz.header.frame_id="base_link";

  pose_viz.pose.position.x = goal_pos_x_+goal_pos_x_offset_;
  pose_viz.pose.position.y = goal_pos_y_+goal_pos_y_offset_;
  pose_viz.pose.position.z = goal_pos_z_+goal_pos_z_offset_;

  double o_x = goal_orient_x_+goal_orient_x_offset_;
  double o_y = goal_orient_y_+goal_orient_y_offset_;
  double o_z = goal_orient_z_+goal_orient_z_offset_;
  pose_viz.pose.orientation.x = o_x;
  pose_viz.pose.orientation.y = o_y;
  pose_viz.pose.orientation.z = o_z; 
  pose_viz.pose.orientation.w = std::sqrt(1.0-(o_x*o_x+o_y*o_y+o_z*o_z));

  goal_pub_.publish(pose_viz);
}

void GraspPanel::updateGoalY()
{
  goal_pos_y_ = (double) (y_spin_->value());

  geometry_msgs::PoseStamped pose_viz;

  pose_viz.header.frame_id="base_link";

  pose_viz.pose.position.x = goal_pos_x_+goal_pos_x_offset_;
  pose_viz.pose.position.y = goal_pos_y_+goal_pos_y_offset_;
  pose_viz.pose.position.z = goal_pos_z_+goal_pos_z_offset_;

  double o_x = goal_orient_x_+goal_orient_x_offset_;
  double o_y = goal_orient_y_+goal_orient_y_offset_;
  double o_z = goal_orient_z_+goal_orient_z_offset_;
  pose_viz.pose.orientation.x = o_x;
  pose_viz.pose.orientation.y = o_y;
  pose_viz.pose.orientation.z = o_z; 
  pose_viz.pose.orientation.w = std::sqrt(1.0-(o_x*o_x+o_y*o_y+o_z*o_z));

  goal_pub_.publish(pose_viz);

}

void GraspPanel::updateGoalZ()
{

  goal_pos_z_ = (double) (z_spin_->value());

  geometry_msgs::PoseStamped pose_viz;

  pose_viz.header.frame_id="base_link";

  pose_viz.pose.position.x = goal_pos_x_+goal_pos_x_offset_;
  pose_viz.pose.position.y = goal_pos_y_+goal_pos_y_offset_;
  pose_viz.pose.position.z = goal_pos_z_+goal_pos_z_offset_;

  double o_x = goal_orient_x_+goal_orient_x_offset_;
  double o_y = goal_orient_y_+goal_orient_y_offset_;
  double o_z = goal_orient_z_+goal_orient_z_offset_;
  pose_viz.pose.orientation.x = o_x;
  pose_viz.pose.orientation.y = o_y;
  pose_viz.pose.orientation.z = o_z; 
  pose_viz.pose.orientation.w = std::sqrt(1.0-(o_x*o_x+o_y*o_y+o_z*o_z));

  goal_pub_.publish(pose_viz);
}


void GraspPanel::save( rviz::Config config ) const
{
  rviz::Panel::save( config );
  config.mapSetValue( "Action", output_action_ );
  config.mapSetValue( "Topic", input_topic_ );
}

// Load all configuration data for this panel from the given Config object.
void GraspPanel::load( const rviz::Config& config )
{
  rviz::Panel::load( config );
  QString action;
  QString topic;
  if( config.mapGetString( "Action", &action ))
  {
    output_action_editor_->setText( action );
    updateAction();
  }

  if( config.mapGetString( "Topic", &topic ))
  {
    input_topic_editor_->setText( topic );
    updateTopic();
  }
}

Marker GraspPanel::makeEndEffector( InteractiveMarker &msg )
{
  Marker marker;

  marker.type = Marker::ARROW;
  marker.scale.x = msg.scale * 0.45;
  marker.scale.y = msg.scale * 0.45;
  marker.scale.z = msg.scale * 0.45;
  marker.color.r = 0.5;
  marker.color.g = 0.5;
  marker.color.b = 0.5;
  marker.color.a = 1.0;

  return marker;
}

InteractiveMarkerControl& GraspPanel::makeEndEffectorControl( InteractiveMarker &msg )
{
  InteractiveMarkerControl control;
  control.always_visible = true;
  control.markers.push_back( makeEndEffector(msg) );
  msg.controls.push_back( control );

  return msg.controls.back();
}

}

#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS(vizzy_rviz_plugins::GraspPanel, rviz::Panel)
