#include "active_3d_planning/module/trajectory_generator.h"

#include "active_3d_planning/module/module_factory.h"
#include "active_3d_planning/planner/planner_I.h"

namespace active_3d_planning {

TrajectoryGenerator::TrajectoryGenerator(PlannerI &planner)
    : Module(planner), voxblox_(planner.getMap()) {}

void TrajectoryGenerator::setupFromParamMap(Module::ParamMap *param_map) {
  setParam<bool>(param_map, "collision_optimistic", &p_collision_optimistic_,
                 false);
  setParam<double>(param_map, "collision_radius", &p_collision_radius_, 0.35);
  setParam<double>(param_map, "clearing_radius", &p_clearing_radius_, 0.0);
  std::string ns = (*param_map)["param_namespace"];
  setParam<std::string>(param_map, "segment_selector_args", &p_selector_args_,
                        ns + "/segment_selector");
  setParam<std::string>(param_map, "generator_updater_args", &p_updater_args_,
                        ns + "/generator_updater");
  std::string temp_args;
  setParam<std::string>(param_map, "bounding_volume_args", &temp_args,
                        ns + "/bounding_volume");
  bounding_volume_ =
      planner_.getFactory().createModule<defaults::BoundingVolume>(
          temp_args, planner_, verbose_modules_);
  setParam<std::string>(param_map, "system_constraints_args", &temp_args,
                        ns + "/system_constraints");
  system_constraints_ =
      planner_.getFactory().createModule<defaults::SystemConstraints>(
          temp_args, planner_, verbose_modules_);
}

bool TrajectoryGenerator::checkTraversable(const Eigen::Vector3d &position) {
  if (!bounding_volume_->contains(position)) {
    return false;
  }
  double distance = 0.0;
  if (voxblox_.getEsdfMapPtr()->getDistanceAtPosition(position, &distance)) {
    // This means the voxel is observed
    return (distance > p_collision_radius_);
  }
  if (p_clearing_radius_ > 0.0) {
    if ((planner_.getCurrentPosition() - position).norm() <
        p_clearing_radius_) {
      return true;
    }
  }
  return p_collision_optimistic_;
}

bool TrajectoryGenerator::selectSegment(TrajectorySegment **result,
                                        TrajectorySegment *root) {
  // If not implemented use a (default) module
  if (!segment_selector_) {
    segment_selector_ = planner_.getFactory().createModule<SegmentSelector>(
        p_selector_args_, planner_, verbose_modules_);
  }
  return segment_selector_->selectSegment(result, root);
}

bool TrajectoryGenerator::updateSegments(TrajectorySegment *root) {
  // If not implemented use a (default) module
  if (!generator_updater_) {
    generator_updater_ = planner_.getFactory().createModule<GeneratorUpdater>(
        p_updater_args_, planner_, verbose_modules_);
  }
  return generator_updater_->updateSegments(root);
}

bool TrajectoryGenerator::extractTrajectoryToPublish(
    EigenTrajectoryPointVector *trajectory, const TrajectorySegment &segment) {
  // Default does not manipulate the stored trajectory
  *trajectory = segment.trajectory;
  return true;
}

SegmentSelector::SegmentSelector(PlannerI &planner) : Module(planner) {}
GeneratorUpdater::GeneratorUpdater(PlannerI &planner) : Module(planner) {}
} // namespace active_3d_planning