#include "robot_models/hyrodyn/RobotModelHyrodyn.hpp"
#include <boost/test/unit_test.hpp>
#include "../test_robot_model.hpp"

using namespace std;
using namespace wbc;

BOOST_AUTO_TEST_CASE(configure_and_update){

    /**
     * Verify that the robot model fails to configure with invalid configurations
     */

    RobotModelConfig config;
    RobotModelHyrodyn robot_model;

    std::vector<std::string> joint_names = {"kuka_lbr_l_joint_1",
                                            "kuka_lbr_l_joint_2",
                                            "kuka_lbr_l_joint_3",
                                            "kuka_lbr_l_joint_4",
                                            "kuka_lbr_l_joint_5",
                                            "kuka_lbr_l_joint_6",
                                            "kuka_lbr_l_joint_7"};
    std::vector<std::string> floating_base_names = {"floating_base_trans_x", "floating_base_trans_y", "floating_base_trans_z",
                                                    "floating_base_rot_x", "floating_base_rot_y", "floating_base_rot_z"};

    // Valid config
    config = RobotModelConfig("../../../../models/kuka/urdf/kuka_iiwa.urdf");
    config.submechanism_file = "../../../../models/kuka/hyrodyn/kuka_iiwa.yml";
    BOOST_CHECK(robot_model.configure(config) == true);
    for(size_t i = 0; i < robot_model.noOfJoints(); i++){
        BOOST_CHECK(joint_names[i] == robot_model.jointNames()[i]);
        BOOST_CHECK(joint_names[i] == robot_model.actuatedJointNames()[i]);
        BOOST_CHECK(joint_names[i] == robot_model.independentJointNames()[i]);
        BOOST_CHECK(i == robot_model.jointIndex(joint_names[i]));
    }

    base::samples::Joints joint_state_in = makeRandomJointState(joint_names);
    BOOST_CHECK_NO_THROW(robot_model.update(joint_state_in));
    base::samples::Joints joint_state_out = robot_model.jointState(joint_names);
    for(auto n : joint_names){
        BOOST_CHECK(joint_state_out[n].position = joint_state_in[n].position);
        BOOST_CHECK(joint_state_out[n].speed = joint_state_in[n].speed);
        BOOST_CHECK(joint_state_out[n].acceleration = joint_state_in[n].acceleration);
    }

    // Invalid filename
    config = RobotModelConfig("../../../../models/kuka/urdf/kuka_iiwa.urd");
    BOOST_CHECK(robot_model.configure(config) == false);

    // Invalid submechanism file
    config = RobotModelConfig("../../../../models/kuka/urdf/kuka_iiwa.urdf");
    config.submechanism_file = "../../../../models/kuka/hyrodyn/kuka_iiwa.ym";
    BOOST_CHECK(robot_model.configure(config) == false);

    // Valid config with floating base
    config = RobotModelConfig("../../../../models/kuka/urdf/kuka_iiwa.urdf");
    config.submechanism_file = "../../../../models/kuka/hyrodyn/kuka_iiwa_floating_base.yml";
    config.floating_base = true;
    BOOST_CHECK(robot_model.configure(config) == true);
    for(size_t i = 0; i < 6; i++){
        BOOST_CHECK(floating_base_names[i] == robot_model.jointNames()[i]);
        BOOST_CHECK(floating_base_names[i] == robot_model.independentJointNames()[i]);
        BOOST_CHECK(i == robot_model.jointIndex(floating_base_names[i]));
    }
    for(size_t i = 0; i < robot_model.noOfActuatedJoints(); i++){
        BOOST_CHECK(joint_names[i] == robot_model.jointNames()[i+6]);
        BOOST_CHECK(joint_names[i] == robot_model.actuatedJointNames()[i]);
        BOOST_CHECK(joint_names[i] == robot_model.independentJointNames()[i+6]);
        BOOST_CHECK(i+6 == robot_model.jointIndex(joint_names[i]));
    }

    base::samples::RigidBodyStateSE3 floating_base_state_in = makeRandomFloatingBaseState();
    BOOST_CHECK_NO_THROW(robot_model.update(joint_state_in, floating_base_state_in));

    base::samples::RigidBodyStateSE3 floating_base_state_out = robot_model.floatingBaseState();
    joint_state_out = robot_model.jointState(joint_names);
    for(auto n : joint_names){
        BOOST_CHECK(joint_state_out[n].position = joint_state_in[n].position);
        BOOST_CHECK(joint_state_out[n].speed = joint_state_in[n].speed);
        BOOST_CHECK(joint_state_out[n].acceleration = joint_state_in[n].acceleration);
    }
    for(int i = 0; i < 3; i++){
        BOOST_CHECK(floating_base_state_in.pose.position[i] == floating_base_state_out.pose.position[i]);
        BOOST_CHECK(floating_base_state_in.pose.orientation.coeffs()[i] == floating_base_state_out.pose.orientation.coeffs()[i]);
        BOOST_CHECK(floating_base_state_in.twist.linear[i] == floating_base_state_out.twist.linear[i]);
        BOOST_CHECK(floating_base_state_in.twist.angular[i] == floating_base_state_out.twist.angular[i]);
        BOOST_CHECK(floating_base_state_in.acceleration.linear[i] == floating_base_state_out.acceleration.linear[i]);
        BOOST_CHECK(floating_base_state_in.acceleration.angular[i] == floating_base_state_out.acceleration.angular[i]);
    }

    // Config with contact points
    config = RobotModelConfig("../../../../models/kuka/urdf/kuka_iiwa.urdf");
    config.submechanism_file = "../../../../models/kuka/hyrodyn/kuka_iiwa.yml";
    config.contact_points.names.push_back("kuka_lbr_l_tcp");
    config.contact_points.elements.push_back(wbc::ActiveContact(1,0.6));
    BOOST_CHECK(robot_model.configure(config) == true);

    // Config with invalid contact points
    config = RobotModelConfig("../../../../models/kuka/urdf/kuka_iiwa.urdf");
    config.submechanism_file = "../../../../models/kuka/hyrodyn/kuka_iiwa.yml";
    config.contact_points.names.push_back("XYZ");
    config.contact_points.elements.push_back(wbc::ActiveContact(1,0.6));
    BOOST_CHECK(robot_model.configure(config) == false);
}


BOOST_AUTO_TEST_CASE(fk){
    string urdf_file = "../../../../models/kuka/urdf/kuka_iiwa.urdf";
    string tip_frame = "kuka_lbr_l_tcp";

    RobotModelPtr robot_model = make_shared<RobotModelHyrodyn>();
    RobotModelConfig cfg(urdf_file);
    cfg.submechanism_file = "../../../../models/kuka/hyrodyn/kuka_iiwa_floating_base.yml";
    cfg.floating_base = true;
    BOOST_CHECK(robot_model->configure(cfg));

    testSpaceJacobian(robot_model, tip_frame, false);
}


BOOST_AUTO_TEST_CASE(space_jacobian){
    string urdf_file = "../../../../models/kuka/urdf/kuka_iiwa.urdf";
    string tip_frame = "kuka_lbr_l_tcp";

    RobotModelPtr robot_model = make_shared<RobotModelHyrodyn>();
    RobotModelConfig cfg(urdf_file);
    cfg.submechanism_file = "../../../../models/kuka/hyrodyn/kuka_iiwa_floating_base.yml";
    cfg.floating_base = true;
    BOOST_CHECK(robot_model->configure(cfg));

    testSpaceJacobian(robot_model, tip_frame, false);
}

BOOST_AUTO_TEST_CASE(body_jacobian){
    string urdf_file = "../../../../models/kuka/urdf/kuka_iiwa.urdf";
    string tip_frame = "kuka_lbr_l_tcp";

    RobotModelPtr robot_model = make_shared<RobotModelHyrodyn>();
    RobotModelConfig cfg(urdf_file);
    cfg.submechanism_file = "../../../../models/kuka/hyrodyn/kuka_iiwa_floating_base.yml";
    cfg.floating_base = true;
    BOOST_CHECK(robot_model->configure(cfg));

    testBodyJacobian(robot_model, tip_frame, false);
}

BOOST_AUTO_TEST_CASE(com_jacobian){
    string urdf_file = "../../../../models/kuka/urdf/kuka_iiwa.urdf";
    string tip_frame = "kuka_lbr_l_tcp";

    RobotModelPtr robot_model = make_shared<RobotModelHyrodyn>();
    RobotModelConfig cfg(urdf_file);
    cfg.submechanism_file = "../../../../models/kuka/hyrodyn/kuka_iiwa.yml";
    cfg.floating_base = false;
    BOOST_CHECK(robot_model->configure(cfg));

    testCoMJacobian(robot_model, false);
}

BOOST_AUTO_TEST_CASE(dynamics){

    string urdf_file = "../../../../models/kuka/urdf/kuka_iiwa.urdf";

    RobotModelPtr robot_model = make_shared<RobotModelHyrodyn>();
    RobotModelConfig cfg(urdf_file);
    cfg.submechanism_file = "../../../../models/kuka/hyrodyn/kuka_iiwa.yml";
    cfg.floating_base = false;
    BOOST_CHECK(robot_model->configure(cfg));

    testDynamics(robot_model, false);

}

BOOST_AUTO_TEST_CASE(compare_serial_vs_hybrid_model){

    /**
     * Check if the differential inverse kinematics solution of a serial robot and the equivalent a series-parallel hybrid robot model match
     */

    string root = "RH5_Root_Link";
    string tip  = "LLAnklePitch_Link";

    RobotModelHyrodyn robot_model_hybrid;
    RobotModelConfig config_hybrid("../../../../models/rh5/urdf/rh5_single_leg_hybrid.urdf");
    config_hybrid.submechanism_file = "../../../../models/rh5/hyrodyn/rh5_single_leg_hybrid.yml";
    BOOST_CHECK(robot_model_hybrid.configure(config_hybrid) == true);


    RobotModelHyrodyn robot_model_serial;
    RobotModelConfig config_serial("../../../../models/rh5/urdf/rh5_single_leg.urdf");
    config_serial.submechanism_file = "../../../../models/rh5/hyrodyn/rh5_single_leg.yml";
    BOOST_CHECK(robot_model_serial.configure(config_serial) == true);

    base::samples::Joints joint_state;
    joint_state.names = robot_model_hybrid.hyrodynHandle()->jointnames_independent;
    for(auto n : robot_model_hybrid.hyrodynHandle()->jointnames_independent){
        base::JointState js;
        js.position = js.speed = js.acceleration = 0;
        joint_state.elements.push_back(js);
    }
    joint_state.time = base::Time::now();
    joint_state["LLKnee"].position = 1.5;
    joint_state["LLAnklePitch"].position = -0.7;

    robot_model_serial.update(joint_state);
    robot_model_hybrid.update(joint_state);

    //cout<<"******************** HYBRID MODEL *****************"<<endl;
    base::MatrixXd jac = robot_model_hybrid.spaceJacobian(root, tip);
    base::Vector6d v;
    v.setZero();
    v[2] = -0.1;
    base::VectorXd u = jac.completeOrthogonalDecomposition().pseudoInverse()*v;
    robot_model_hybrid.hyrodynHandle()->ud = u;
    robot_model_hybrid.hyrodynHandle()->calculate_forward_system_state();

    /*cout<< "Solution actuation space" << endl;
    std::cout<<robot_model_hybrid.hyrodynHandle()->ud.transpose()<<endl;

    cout<< "Solution projected to independent joint space" << endl;
    std::cout<<robot_model_hybrid.hyrodynHandle()->yd.transpose()<<endl;*/

    //cout<<"******************** SERIAL MODEL *****************"<<endl;
    jac = robot_model_serial.spaceJacobian(root, tip);
    base::VectorXd yd = jac.completeOrthogonalDecomposition().pseudoInverse()*v;

    /*cout<< "Solution independent joint space" << endl;
    std::cout<<yd.transpose()<<endl;*/

    for(int i = 0; i < robot_model_hybrid.noOfActuatedJoints(); i++)
        BOOST_CHECK(fabs(robot_model_hybrid.hyrodynHandle()->yd[i] - yd[i]) < 1e-6);
}
