#ifndef MAV_ACTIVE_3D_PLANNING_TRAJECTORY_GENERATORS_RRT_STAR_H
#define MAV_ACTIVE_3D_PLANNING_TRAJECTORY_GENERATORS_RRT_STAR_H

#include "mav_active_3d_planning/modules/trajectory_generators/rrt.h"
#include "mav_active_3d_planning/trajectory_evaluator.h"
#include "mav_active_3d_planning/planner_node.h"


namespace mav_active_3d_planning {
    namespace trajectory_evaluators { class RRTStarEvaluatorAdapter; }
    namespace trajectory_generators {

        // Extension of the RRT class. Use with its trajectory evaluator adapter below to find the gain, cost and value
        // for new segments and tries rewiring the best segments together. RRTStar requires trajectory evaluation based
        // only on the last point, since it evaluates and connects points (and not trajectories).
        class RRTStar : public RRT {
        public:
            // override virtual functions
            virtual bool expandSegment(TrajectorySegment *target, std::vector<TrajectorySegment *> *new_segments);

            bool rewireRoot(TrajectorySegment *root, int *next_segment);

        protected:
            friend ModuleFactory;
            friend mav_active_3d_planning::trajectory_evaluators::RRTStarEvaluatorAdapter;

            // factory access
            RRTStar() {}

            virtual void setupFromParamMap(Module::ParamMap *param_map);

            virtual bool checkParamsValid(std::string *error_message);

            static ModuleFactory::Registration<RRTStar> registration;

            // parameters
            bool p_rewire_root_;            // if true try rewiring the children of the current segment (to keep them alive)
            bool p_rewire_intermediate_;    // If true try rewiring existing segments to new candidates if this improves their value
            double p_max_rewire_range_;     // distance [m] within which rewiring is possible
            int p_n_neighbors_;             // How many knns to consider for rewiring

            // constants
            double c_rewire_range_square_;

            // variables
            bool tree_is_reset_;            // Force reset of kdtree if requestNext is called twice (dangling pointers)

            // Pointer to planner node to call for evaluation
            PlannerNode *planner_node_;

            // methods
            bool findNearbyCandidates(const Eigen::Vector3d &target_point, std::vector<TrajectorySegment *> *result);

            bool rewireToBestParent(TrajectorySegment *segment, const std::vector <TrajectorySegment*> &candidates);
        };

    } // namespace trajectory_generators

    namespace trajectory_evaluators {

        // RRTStar Evaluator adapter, its following evaluator is used for all regular evaluation duties.
        class RRTStarEvaluatorAdapter : public TrajectoryEvaluator {
        public:

            // Override virtual functions
            bool computeGain(TrajectorySegment *traj_in);

            bool computeCost(TrajectorySegment *traj_in);

            bool computeValue(TrajectorySegment *traj_in);

            int selectNextBest(TrajectorySegment *traj_in);

            bool updateSegments(TrajectorySegment *root);

            void visualizeTrajectoryValue(visualization_msgs::MarkerArray *msg, const TrajectorySegment &trajectory);

        protected:
            friend ModuleFactory;

            // factory access
            RRTStarEvaluatorAdapter() {}

            void setupFromParamMap(Module::ParamMap *param_map);

            static ModuleFactory::Registration<RRTStarEvaluatorAdapter> registration;

            // members
            std::unique_ptr <TrajectoryEvaluator> following_evaluator_;
            mav_active_3d_planning::trajectory_generators::RRTStar *generator_;
        };

    } //namespace trajectory_evaluators
}  // namespace mav_active_3d_planning
#endif // MAV_ACTIVE_3D_PLANNING_TRAJECTORY_GENERATORS_RRT_STAR_H
