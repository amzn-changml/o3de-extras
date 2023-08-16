/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/UnitTest/TestTypes.h>
#include <AzCore/std/ranges/ranges_algorithm.h>
#include <AzCore/std/string/string.h>
#include <AzTest/AzTest.h>
#include <RobotImporter/URDF/UrdfParser.h>
#include <RobotImporter/Utils/RobotImporterUtils.h>
#include <RobotImporter/xacro/XacroUtils.h>

namespace UnitTest
{

    class UrdfParserTest : public LeakDetectionFixture
    {
    public:
        AZStd::string GetXacroParams()
        {
            return "<robot name=\"test\" xmlns:xacro=\"http://ros.org/wiki/xacro\">\n"
                   "    <xacro:arg name=\"laser_enabled\" default=\"false\" />\n"
                   "</robot>";
        }
        AZStd::string GetUrdfWithOneLink()
        {
            return "<robot name=\"test_one_link\">"
                   "  <material name=\"some_material\">\n"
                   "    <color rgba=\"0 0 0 1\"/>\n"
                   "  </material>"
                   "  <link name=\"link1\">"
                   "    <inertial>"
                   "      <mass value=\"1.0\"/>"
                   "      <inertia ixx=\"1.0\" iyy=\"1.0\" izz=\"1.0\" ixy=\"0\" ixz=\"0\" iyz=\"0\"/>"
                   "    </inertial>"
                   "    <visual>"
                   "      <geometry>"
                   "        <box size=\"1.0 2.0 1.0\"/>"
                   "      </geometry>"
                   "      <material name=\"some_material\"/>"
                   "    </visual>"
                   "    <collision>"
                   "      <geometry>"
                   "        <box size=\"1.0 2.0 1.0\"/>"
                   "      </geometry>"
                   "    </collision>"
                   "  </link>"
                   "</robot>";
        }

        AZStd::string GetUrdfWithTwoLinksAndJoint(AZStd::string_view jointType = "fixed")
        {
            return AZStd::string::format("<robot name=\"test_two_links_one_joint\">  "
                   "  <material name=\"some_material\">\n"
                   "    <color rgba=\"0 0 0 1\"/>\n"
                   "  </material>"
                   "  <link name=\"link1\">"
                   "    <inertial>"
                   "      <mass value=\"1.0\"/>"
                   "      <inertia ixx=\"1.0\" iyy=\"1.0\" izz=\"1.0\" ixy=\"0\" ixz=\"0\" iyz=\"0\"/>"
                   "    </inertial>"
                   "    <visual>"
                   "      <geometry>"
                   "        <box size=\"1.0 2.0 1.0\"/>"
                   "      </geometry>"
                   "      <material name=\"some_material\"/>"
                   "    </visual>"
                   "  </link>"
                   "  <link name=\"link2\">"
                   "    <inertial>"
                   "      <mass value=\"1.0\"/>"
                   "      <inertia ixx=\"1.0\" iyy=\"1.0\" izz=\"1.0\" ixy=\"0\" ixz=\"0\" iyz=\"0\"/>"
                   "    </inertial>"
                   "    <visual>"
                   "      <geometry>"
                   "        <box size=\"1.0 1.0 1.0\"/>"
                   "      </geometry>"
                   "      <material name=\"some_material\"/>"
                   "    </visual>"
                   "  </link>"
                   R"(  <joint name="joint12" type="%.*s">)"
                   "    <parent link=\"link1\"/>"
                   "    <child link=\"link2\"/>"
                   "    <origin rpy=\"0 0 0\" xyz=\"1.0 0.5 0.0\"/>"
                   "    <dynamics damping=\"10.0\" friction=\"5.0\"/>"
                   "    <limit lower=\"10.0\" upper=\"20.0\" effort=\"90.0\" velocity=\"10.0\"/>"
                   "  </joint>"
                   "</robot>", AZ_STRING_ARG(jointType));
        }

        // A URDF <model> can only represent structure which is configurable though the <joint> tags
        // Therefore link can only appear as a child of a single joint.
        // It cannot be a child of multiple joints
        // https://wiki.ros.org/urdf/XML/model
        AZStd::string GetURDFWithTranforms()
        {
            return "<?xml version=\"1.0\"?>\n"
                   "<robot name=\"complicated\">\n"
                   "    <link name=\"base_link\">\n"
                   "    </link>\n"
                   "    <link name=\"link1\">\n"
                   "        <inertial>\n"
                   "        <origin xyz=\"0. 0. 0.\"/>\n"
                   "        <mass value=\"1.\"/>\n"
                   "        <inertia ixx=\"1.\" ixy=\"0.\" ixz=\"0.\" iyy=\"1.\" iyz=\"0.\" izz=\"1.\"/>\n"
                   "        </inertial>\n"
                   "        <visual>\n"
                   "            <origin rpy=\"0.000000 -0.000000 0\" xyz=\"-1.2 0 0\"/>\n"
                   "            <geometry>\n"
                   "                <box size=\"2.000000 0.200000 0.200000\"/>\n"
                   "            </geometry>\n"
                   "        </visual>\n"
                   "        <collision>\n"
                   "            <origin rpy=\"0.000000 -0.000000 0\" xyz=\"-1.2 0 0.000000\"/>\n"
                   "            <geometry>\n"
                   "                <box size=\"2.000000 0.200000 0.200000\"/>\n"
                   "            </geometry>            \n"
                   "        </collision>\n"
                   "    </link>\n"
                   "    <link name=\"link2\">\n"
                   "        <inertial>\n"
                   "        <origin xyz=\"0. 0. 0.\"/>\n"
                   "        <mass value=\"1.\"/>\n"
                   "        <inertia ixx=\"1.\" ixy=\"0.\" ixz=\"0.\" iyy=\"1.\" iyz=\"0.\" izz=\"1.\"/>\n"
                   "        </inertial>\n"
                   "        <visual>\n"
                   "            <origin rpy=\"0.000000 -0.000000 0\" xyz=\"-1.2 0 0\"/>\n"
                   "            <geometry>\n"
                   "                <box size=\"2.000000 0.200000 0.200000\"/>\n"
                   "            </geometry>\n"
                   "        </visual>\n"
                   "        <collision>\n"
                   "            <origin rpy=\"0.000000 -0.000000 0\" xyz=\"-1.2 0 0.000000\"/>\n"
                   "            <geometry>\n"
                   "                <box size=\"2.000000 0.200000 0.200000\"/>\n"
                   "            </geometry>            \n"
                   "        </collision>\n"
                   "    </link>\n"
                   "    <link name=\"link3\">\n"
                   "        <inertial>\n"
                   "        <origin xyz=\"0. 0. 0.\"/>\n"
                   "        <mass value=\"1.\"/>\n"
                   "        <inertia ixx=\"1.\" ixy=\"0.\" ixz=\"0.\" iyy=\"1.\" iyz=\"0.\" izz=\"1.\"/>\n"
                   "        </inertial>\n"
                   "        <visual>\n"
                   "            <origin rpy=\"0.000000 -0.000000 0\" xyz=\"-1.2 0 0\"/>\n"
                   "            <geometry>\n"
                   "                <box size=\"2.000000 0.200000 0.200000\"/>\n"
                   "            </geometry>\n"
                   "        </visual>\n"
                   "        <collision>\n"
                   "            <origin rpy=\"0.000000 -0.000000 0\" xyz=\"-1.2 0 0.000000\"/>\n"
                   "            <geometry>\n"
                   "                <box size=\"2.000000 0.200000 0.200000\"/>\n"
                   "            </geometry>            \n"
                   "        </collision>\n"
                   "    </link>\n"
                   "    <joint name=\"joint_bs\" type=\"fixed\">\n"
                   "        <parent link=\"base_link\"/>\n"
                   "        <child link=\"link1\"/>\n"
                   "        <axis xyz=\"0. 0. 1.\"/>\n"
                   "        <origin rpy=\"0 0 0\" xyz=\"0 0 0\"/>\n"
                   "    </joint> \n"
                   "    <joint name=\"joint0\" type=\"continuous\">\n"
                   "        <parent link=\"link1\"/>\n"
                   "        <child link=\"link2\"/>\n"
                   "        <axis xyz=\"0. 0. 1.\"/>\n"
                   "        <origin rpy=\"0.000000 -0.000000 2.094395\" xyz=\"-1.200000 2.078460 0.000000\"/>\n"
                   "    </joint> \n"
                   "    <joint name=\"joint1\" type=\"continuous\">\n"
                   "        <parent link=\"link2\"/>\n"
                   "        <child link=\"link3\"/>\n"
                   "        <axis xyz=\"0. 0. 1.\"/>\n"
                   "        <origin rpy=\"0.000000 0.000000 2.094395\" xyz=\"-1.200000286102295 2.078460931777954 0.\"/>\n"
                   "    </joint> \n"
                   "</robot>";
        }

        AZStd::string GetURDFWithWheel(
            const AZStd::string& wheel_name,
            const AZStd::string& wheel_joint_type,
            bool wheel_has_visual = true,
            bool wheel_has_collider = true)
        {
            // clang-format off
            return "<robot name=\"wheel_test\">\n"
                   "    <link name=\"base_link\">\n"
                   "        <inertial>\n"
                   "            <origin xyz=\"0. 0. 0.\"/>\n"
                   "            <mass value=\"1.\"/>\n"
                   "            <inertia ixx=\"1.\" ixy=\"0.\" ixz=\"0.\" iyy=\"1.\" iyz=\"0.\" izz=\"1.\"/>\n"
                   "        </inertial>\n"
                   "    </link>\n"
                   "    <link name=\""+wheel_name+"\">\n"
                   "        <inertial>\n"
                   "            <origin xyz=\"0. 0. 0.\"/>\n"
                   "            <mass value=\"1.\"/>\n"
                   "            <inertia ixx=\"1.\" ixy=\"0.\" ixz=\"0.\" iyy=\"1.\" iyz=\"0.\" izz=\"1.\"/>\n"
                   "        </inertial>\n"
                   +(wheel_has_visual?"<visual>\n"
                   "            <origin rpy=\"0 0 0\" xyz=\"0 0 0\"/>\n"
                   "            <geometry>\n"
                   "                <box size=\"1 1 1\"/>\n"
                   "            </geometry>\n"
                   "        </visual>\n":"")+
                   +(wheel_has_collider?"<collision>\n"
                   "            <origin rpy=\"0 0 0\" xyz=\"0 0 0\"/>\n"
                   "            <geometry>\n"
                   "                <box size=\"1 1 1\"/>\n"
                   "            </geometry>\n"
                   "        </collision>\n":"")+
                   "    </link>\n"
                   "    <joint name=\"joint0\" type=\""+wheel_joint_type+"\">\n"
                   "        <parent link=\"base_link\"/>\n"
                   "        <child link=\""+wheel_name+"\"/>\n"
                   "        <axis xyz=\"0. 0. 1.\"/>\n"
                   "        <origin rpy=\"0. 0. 0.\" xyz=\"2. 0. 0.\"/>\n"
                   "    </joint>\n"
                   "</robot>";
            // clang-format on
        }
    };

    TEST_F(UrdfParserTest, ParseUrdfWithOneLink)
    {
        const auto xmlStr = GetUrdfWithOneLink();
        const auto sdfRootOutcome = ROS2::UrdfParser::Parse(xmlStr);
        ASSERT_TRUE(sdfRootOutcome);
        const sdf::Root& sdfRoot = sdfRootOutcome.value();

        const sdf::Model* model = sdfRoot.Model();
        EXPECT_EQ("test_one_link", model->Name());
        ASSERT_NE(nullptr, model);

        uint64_t linkCount = model->LinkCount();
        EXPECT_EQ(1U, linkCount);

        const sdf::Link* link1 = model->LinkByName("link1");

        ASSERT_NE(nullptr, link1);
        EXPECT_EQ(1.0, link1->Inertial().MassMatrix().Mass());
        EXPECT_EQ(1.0, link1->Inertial().MassMatrix().Ixx());
        EXPECT_EQ(0.0, link1->Inertial().MassMatrix().Ixy());
        EXPECT_EQ(0.0, link1->Inertial().MassMatrix().Ixz());
        EXPECT_EQ(1.0, link1->Inertial().MassMatrix().Iyy());
        EXPECT_EQ(0.0, link1->Inertial().MassMatrix().Iyz());
        EXPECT_EQ(1.0, link1->Inertial().MassMatrix().Izz());

        ASSERT_EQ(1, link1->VisualCount());
        const sdf::Visual* visual = link1->VisualByIndex(0);
        ASSERT_NE(nullptr, visual);
        const sdf::Geometry* geometryVis = visual->Geom();
        ASSERT_NE(nullptr, geometryVis);
        EXPECT_EQ(sdf::GeometryType::BOX, geometryVis->Type());
        const sdf::Box* visualBox = geometryVis->BoxShape();
        ASSERT_NE(nullptr, visualBox);
        EXPECT_EQ(1.0, visualBox->Size().X());
        EXPECT_EQ(2.0, visualBox->Size().Y());
        EXPECT_EQ(1.0, visualBox->Size().Z());

        ASSERT_EQ(1, link1->CollisionCount());
        const sdf::Collision* collision = link1->CollisionByIndex(0);
        ASSERT_NE(nullptr, collision);
        const sdf::Geometry* geometryCol = visual->Geom();
        ASSERT_NE(nullptr, geometryCol);
        EXPECT_EQ(sdf::GeometryType::BOX, geometryCol->Type());
        const sdf::Box* collisionBox = geometryCol->BoxShape();
        EXPECT_EQ(1.0, collisionBox->Size().X());
        EXPECT_EQ(2.0, collisionBox->Size().Y());
        EXPECT_EQ(1.0, collisionBox->Size().Z());
    }

    TEST_F(UrdfParserTest, ParseUrdfWithTwoLinksAndFixedJoint)
    {
        const auto xmlStr = GetUrdfWithTwoLinksAndJoint();
        const auto sdfRootOutcome = ROS2::UrdfParser::Parse(xmlStr);
        ASSERT_TRUE(sdfRootOutcome);
        const sdf::Root& sdfRoot = sdfRootOutcome.value();

        const sdf::Model* model = sdfRoot.Model();
        EXPECT_EQ("test_two_links_one_joint", model->Name());
        ASSERT_NE(nullptr, model);

        // The SDFormat URDF parser combines links in joints that are fixed
        // together
        // https://github.com/gazebosim/sdformat/pull/1149
        // So for a URDF with 2 links that are combined with a single fixed joint
        // The resulted SDF has one 1 link and no joints
        //
        // The SDFormat <gazebo> extension tag can be used to preserve fixed joint by adding
        // a <gazebo><preserveFixedJoint>true</preserveFixedJoint></gazebo> XML element
        // http://sdformat.org/tutorials?tut=sdformat_urdf_extensions&cat=specification&#gazebo-elements-for-joints
        ASSERT_EQ(1, model->LinkCount());

        EXPECT_TRUE(model->FrameNameExists("link2"));
        EXPECT_TRUE(model->FrameNameExists("joint12"));

        const sdf::Link* link1 = model->LinkByName("link1");
        ASSERT_NE(nullptr, link1);
    }

    TEST_F(UrdfParserTest, ParseUrdfWithTwoLinksAndNonFixedJoint)
    {
        const auto xmlStr = GetUrdfWithTwoLinksAndJoint("continuous");
        const auto sdfRootOutcome = ROS2::UrdfParser::Parse(xmlStr);
        ASSERT_TRUE(sdfRootOutcome);
        const sdf::Root& sdfRoot = sdfRootOutcome.value();

        const sdf::Model* model = sdfRoot.Model();
        EXPECT_EQ("test_two_links_one_joint", model->Name());
        ASSERT_NE(nullptr, model);

        ASSERT_EQ(2, model->LinkCount());

        const sdf::Link* link1 = model->LinkByName("link1");
        ASSERT_NE(nullptr, link1);

        const sdf::Link* link2 = model->LinkByName("link2");
        ASSERT_NE(nullptr, link2);

        const sdf::Joint* joint12 = model->JointByName("joint12");
        ASSERT_NE(nullptr, joint12);

        EXPECT_EQ("link1", joint12->ParentName());
        EXPECT_EQ("link2", joint12->ChildName());

        gz::math::Pose3d jointPose;
        sdf::Errors poseResolveErrors = joint12->SemanticPose().Resolve(jointPose);
        EXPECT_TRUE(poseResolveErrors.empty());
        EXPECT_EQ(0.0, jointPose.X());
        EXPECT_EQ(0.0, jointPose.Y());
        EXPECT_EQ(0.0, jointPose.Z());

        double roll, pitch, yaw;
        const gz::math::Quaternion rot = jointPose.Rot();
        roll = rot.Roll();
        pitch = rot.Pitch();
        yaw = rot.Yaw();
        EXPECT_DOUBLE_EQ(roll, 0.0);
        EXPECT_DOUBLE_EQ(pitch, 0.0);
        EXPECT_DOUBLE_EQ(yaw, 0.0);

        const sdf::JointAxis* joint12Axis = joint12->Axis();
        ASSERT_NE(nullptr, joint12Axis);

        EXPECT_EQ(10.0, joint12Axis->Damping());
        EXPECT_EQ(5.0, joint12Axis->Friction());

        EXPECT_EQ(-AZStd::numeric_limits<double>::infinity(), joint12Axis->Lower());
        EXPECT_EQ(AZStd::numeric_limits<double>::infinity(), joint12Axis->Upper());
        EXPECT_EQ(90.0, joint12Axis->Effort());
        EXPECT_EQ(10.0, joint12Axis->MaxVelocity());
    }

    TEST_F(UrdfParserTest, WheelHeuristicNameValid)
    {
        const AZStd::string_view wheelName("wheel_left_link");
        const auto xmlStr = GetURDFWithWheel(wheelName, "continuous");
        const auto sdfRootOutcome = ROS2::UrdfParser::Parse(xmlStr);
        ASSERT_TRUE(sdfRootOutcome);
        const sdf::Root& sdfRoot = sdfRootOutcome.value();
        const sdf::Model* model = sdfRoot.Model();
        ASSERT_NE(nullptr, model);
        auto wheelCandidate = model->LinkByName(std::string(wheelName.data(), wheelName.size()));
        ASSERT_NE(nullptr, wheelCandidate);
        EXPECT_TRUE(ROS2::Utils::IsWheelURDFHeuristics(*model, wheelCandidate));
    }

    TEST_F(UrdfParserTest, WheelHeuristicNameNotValid1)
    {
        const AZStd::string wheelName("wheel_left_joint");
        const auto xmlStr = GetURDFWithWheel(wheelName, "continuous");
        const auto sdfRootOutcome = ROS2::UrdfParser::Parse(xmlStr);
        ASSERT_TRUE(sdfRootOutcome);
        const sdf::Root& sdfRoot = sdfRootOutcome.value();
        const sdf::Model* model = sdfRoot.Model();
        ASSERT_NE(nullptr, model);
        auto wheelCandidate = model->LinkByName(std::string(wheelName.data(), wheelName.size()));
        ASSERT_NE(nullptr, wheelCandidate);
        EXPECT_FALSE(ROS2::Utils::IsWheelURDFHeuristics(*model, wheelCandidate));
    }

    TEST_F(UrdfParserTest, WheelHeuristicJointNotValid)
    {
        const AZStd::string wheelName("wheel_left_link");
        const auto xmlStr = GetURDFWithWheel(wheelName, "fixed");
        const auto sdfRootOutcome = ROS2::UrdfParser::Parse(xmlStr);
        ASSERT_TRUE(sdfRootOutcome);
        const sdf::Root& sdfRoot = sdfRootOutcome.value();
        const sdf::Model* model = sdfRoot.Model();
        ASSERT_NE(nullptr, model);
        // SDFormat converts combines the links of a joint with a fixed type
        // into a single link
        // It does however create a Frame with the name of the child link and joint that was combined
        EXPECT_EQ(1, model->LinkCount());

        auto wheelCandidate = model->LinkByName("base_link");
        ASSERT_NE(nullptr, wheelCandidate);

        EXPECT_TRUE(model->FrameNameExists(std::string{ wheelName.c_str(), wheelName.size() }));
        EXPECT_TRUE(model->FrameNameExists("joint0"));
        EXPECT_FALSE(ROS2::Utils::IsWheelURDFHeuristics(*model, wheelCandidate));
    }

    TEST_F(UrdfParserTest, WheelHeuristicJointVisualNotValid)
    {
        const AZStd::string wheelName("wheel_left_link");
        const auto xmlStr = GetURDFWithWheel(wheelName, "continuous", false, true);
        const auto sdfRootOutcome = ROS2::UrdfParser::Parse(xmlStr);
        ASSERT_TRUE(sdfRootOutcome);
        const sdf::Root& sdfRoot = sdfRootOutcome.value();
        const sdf::Model* model = sdfRoot.Model();
        ASSERT_NE(nullptr, model);
        auto wheelCandidate = model->LinkByName(std::string(wheelName.c_str(), wheelName.size()));
        ASSERT_NE(nullptr, wheelCandidate);
        EXPECT_FALSE(ROS2::Utils::IsWheelURDFHeuristics(*model, wheelCandidate));
    }

    TEST_F(UrdfParserTest, WheelHeuristicJointColliderNotValid)
    {
        const AZStd::string wheelName("wheel_left_link");
        const auto xmlStr = GetURDFWithWheel(wheelName, "continuous", true, false);
        const auto sdfRootOutcome = ROS2::UrdfParser::Parse(xmlStr);
        ASSERT_TRUE(sdfRootOutcome);
        const sdf::Root& sdfRoot = sdfRootOutcome.value();
        const sdf::Model* model = sdfRoot.Model();
        ASSERT_NE(nullptr, model);
        auto wheelCandidate = model->LinkByName(std::string(wheelName.c_str(), wheelName.size()));
        ASSERT_NE(nullptr, wheelCandidate);
        EXPECT_FALSE(ROS2::Utils::IsWheelURDFHeuristics(*model, wheelCandidate));
    }

    TEST_F(UrdfParserTest, TestLinkListing)
    {
        const auto xmlStr = GetURDFWithTranforms();
        const auto sdfRootOutcome = ROS2::UrdfParser::Parse(xmlStr);
        ASSERT_TRUE(sdfRootOutcome);
        const sdf::Root& sdfRoot = sdfRootOutcome.value();
        const sdf::Model* model = sdfRoot.Model();
        ASSERT_NE(nullptr, model);
        auto links = ROS2::Utils::GetAllLinks(*model);
        // As the "joint_bs" is a fixed joint, it and it's child link are combined
        // Therefore the "link1" child link and "joint_bs" fixed joint are combined
        // into the base_link of the SDF
        // However there are Frames for the combined links and joints
        EXPECT_EQ(links.size(), 3);
        ASSERT_TRUE(links.contains("base_link"));
        ASSERT_TRUE(links.contains("link2"));
        ASSERT_TRUE(links.contains("link3"));
        EXPECT_EQ("base_link", links.at("base_link")->Name());
        EXPECT_EQ("link2", links.at("link2")->Name());
        EXPECT_EQ("link3", links.at("link3")->Name());

        // Check that the frame names exist on the model
        EXPECT_TRUE(model->FrameNameExists("joint_bs"));
        EXPECT_TRUE(model->FrameNameExists("link1"));
    }

    TEST_F(UrdfParserTest, TestJointLink)
    {
        const auto xmlStr = GetURDFWithTranforms();
        const auto sdfRootOutcome = ROS2::UrdfParser::Parse(xmlStr);
        ASSERT_TRUE(sdfRootOutcome);
        const sdf::Root& sdfRoot = sdfRootOutcome.value();
        const sdf::Model* model = sdfRoot.Model();
        ASSERT_NE(nullptr, model);
        auto joints = ROS2::Utils::GetAllJoints(*model);
        EXPECT_EQ(2, joints.size());
        ASSERT_TRUE(joints.contains("joint0"));
        ASSERT_TRUE(joints.contains("joint1"));
    }

    TEST_F(UrdfParserTest, TestTransforms)
    {
        const auto xmlStr = GetURDFWithTranforms();
        const auto sdfRootOutcome = ROS2::UrdfParser::Parse(xmlStr);
        ASSERT_TRUE(sdfRootOutcome);
        const sdf::Root& sdfRoot = sdfRootOutcome.value();
        const sdf::Model* model = sdfRoot.Model();
        ASSERT_NE(nullptr, model);
        const auto links = ROS2::Utils::GetAllLinks(*model);
        // The "link1" is combined with the base_link through
        // joint reduction in the URDF->SDF parser logic
        // https://github.com/gazebosim/sdformat/issues/1110
        ASSERT_TRUE(links.contains("base_link"));
        ASSERT_TRUE(links.contains("link2"));
        ASSERT_TRUE(links.contains("link3"));
        const auto base_link_ptr = links.at("base_link");
        const auto link2_ptr = links.at("link2");
        const auto link3_ptr = links.at("link3");

        // values exported from Blender
        const AZ::Vector3 expected_translation_link1{ 0.0, 0.0, 0.0 };
        const AZ::Vector3 expected_translation_link2{ -1.2000000476837158, 2.0784599781036377, 0.0 };
        const AZ::Vector3 expected_translation_link3{ -2.4000000953674316, 0.0, 0.0 };

        const AZ::Transform transform_from_urdf_link1 = ROS2::Utils::GetWorldTransformURDF(base_link_ptr);
        EXPECT_NEAR(expected_translation_link1.GetX(), transform_from_urdf_link1.GetTranslation().GetX(), 1e-5);
        EXPECT_NEAR(expected_translation_link1.GetY(), transform_from_urdf_link1.GetTranslation().GetY(), 1e-5);
        EXPECT_NEAR(expected_translation_link1.GetZ(), transform_from_urdf_link1.GetTranslation().GetZ(), 1e-5);

        const AZ::Transform transform_from_urdf_link2 = ROS2::Utils::GetWorldTransformURDF(link2_ptr);
        EXPECT_NEAR(expected_translation_link2.GetX(), transform_from_urdf_link2.GetTranslation().GetX(), 1e-5);
        EXPECT_NEAR(expected_translation_link2.GetY(), transform_from_urdf_link2.GetTranslation().GetY(), 1e-5);
        EXPECT_NEAR(expected_translation_link2.GetZ(), transform_from_urdf_link2.GetTranslation().GetZ(), 1e-5);

        const AZ::Transform transform_from_urdf_link3 = ROS2::Utils::GetWorldTransformURDF(link3_ptr);
        EXPECT_NEAR(expected_translation_link3.GetX(), transform_from_urdf_link3.GetTranslation().GetX(), 1e-5);
        EXPECT_NEAR(expected_translation_link3.GetY(), transform_from_urdf_link3.GetTranslation().GetY(), 1e-5);
        EXPECT_NEAR(expected_translation_link3.GetZ(), transform_from_urdf_link3.GetTranslation().GetZ(), 1e-5);
    }

    TEST_F(UrdfParserTest, TestQueryJointsForParentLink_Succeeds)
    {
        const auto xmlStr = GetURDFWithTranforms();
        const auto sdfRootOutcome = ROS2::UrdfParser::Parse(xmlStr);
        ASSERT_TRUE(sdfRootOutcome);
        const sdf::Root& sdfRoot = sdfRootOutcome.value();
        const sdf::Model* model = sdfRoot.Model();
        ASSERT_NE(nullptr, model);
        auto joints = ROS2::Utils::GetJointsForParentLink(*model, "base_link");
        EXPECT_EQ(1, joints.size());

        auto jointToNameProjection = [](const sdf::Joint* joint)
        {
            return AZStd::string_view(joint->Name().c_str(), joint->Name().size());
        };
        ASSERT_TRUE(AZStd::ranges::contains(joints, "joint0", jointToNameProjection));

        // Now check the middle link of "link2"
        joints = ROS2::Utils::GetJointsForParentLink(*model, "link2");
        EXPECT_EQ(1, joints.size());

        ASSERT_TRUE(AZStd::ranges::contains(joints, "joint1", jointToNameProjection));
    }

    TEST_F(UrdfParserTest, TestQueryJointsForChildLink_Succeeds)
    {
        const auto xmlStr = GetURDFWithTranforms();
        const auto sdfRootOutcome = ROS2::UrdfParser::Parse(xmlStr);
        ASSERT_TRUE(sdfRootOutcome);
        const sdf::Root& sdfRoot = sdfRootOutcome.value();
        const sdf::Model* model = sdfRoot.Model();
        ASSERT_NE(nullptr, model);
        auto joints = ROS2::Utils::GetJointsForChildLink(*model, "link2");
        EXPECT_EQ(1, joints.size());

        auto jointToNameProjection = [](const sdf::Joint* joint)
        {
            return AZStd::string_view(joint->Name().c_str(), joint->Name().size());
        };
        ASSERT_TRUE(AZStd::ranges::contains(joints, "joint0", jointToNameProjection));

        // Now check the final link of "link3"
        joints = ROS2::Utils::GetJointsForChildLink(*model, "link3");
        EXPECT_EQ(1, joints.size());

        ASSERT_TRUE(AZStd::ranges::contains(joints, "joint1", jointToNameProjection));
    }

    TEST_F(UrdfParserTest, TestPathResolvementGlobal)
    {
        AZStd::string dae = "file:///home/foo/ros_ws/install/foo_robot/meshes/bar.dae";
        AZStd::string urdf = "/home/foo/ros_ws/install/foo_robot/foo_robot.urdf";
        auto result = ROS2::Utils::ResolveURDFPath(
            dae,
            urdf, "",
            [](const AZStd::string& p) -> bool
            {
                return false;
            });
        EXPECT_EQ(result, "/home/foo/ros_ws/install/foo_robot/meshes/bar.dae");
    }

    TEST_F(UrdfParserTest, TestPathResolvementRelative)
    {
        AZStd::string dae = "meshes/bar.dae";
        AZStd::string urdf = "/home/foo/ros_ws/install/foo_robot/foo_robot.urdf";
        auto result = ROS2::Utils::ResolveURDFPath(
            dae,
            urdf, "",
            [](const AZStd::string& p) -> bool
            {
                return false;
            });
        EXPECT_EQ(result, "/home/foo/ros_ws/install/foo_robot/meshes/bar.dae");
    }

    TEST_F(UrdfParserTest, TestPathResolvementRelativePackage)
    {
        AZStd::string dae = "package://meshes/bar.dae";
        AZStd::string urdf = "/home/foo/ros_ws/install/foo_robot/description/foo_robot.urdf";
        AZStd::string xml = "/home/foo/ros_ws/install/foo_robot/package.xml";
        AZStd::string resolvedDae = "/home/foo/ros_ws/install/foo_robot/meshes/bar.dae";
        auto mockFileSystem = [&](const AZStd::string& p) -> bool
        {
            return (p == xml || p == resolvedDae);
        };
        auto result = ROS2::Utils::ResolveURDFPath(dae, urdf, "", mockFileSystem);
        EXPECT_EQ(result, resolvedDae);
    }

    TEST_F(UrdfParserTest, TestPathResolvementExplicitPackageName)
    {
        AZStd::string dae = "package://foo_robot/meshes/bar.dae";
        AZStd::string urdf = "/home/foo/ros_ws/install/foo_robot/share/foo_robot/description/foo_robot.urdf";
        AZStd::string xml = "/home/foo/ros_ws/install/foo_robot/share/foo_robot/package.xml";
        AZStd::string resolvedDae = "/home/foo/ros_ws/install/foo_robot/share/foo_robot/meshes/bar.dae";
        auto mockFileSystem = [&](const AZStd::string& p) -> bool
        {
            return (p == xml || p == resolvedDae);
        };
        auto result = ROS2::Utils::ResolveURDFPath(dae, urdf, "/home/foo/ros_ws/install/foo_robot", mockFileSystem);
        EXPECT_EQ(result, resolvedDae);
    }

    TEST_F(UrdfParserTest, XacroParseArgsInvalid)
    {
        AZStd::string xacroParams = GetXacroParams();
        ROS2::Utils::xacro::Params params = ROS2::Utils::xacro::GetParameterFromXacroData("");
        EXPECT_EQ(params.size(), 0);
    }

    TEST_F(UrdfParserTest, XacroParseArgs)
    {
        AZStd::string xacroParams = GetXacroParams();
        ROS2::Utils::xacro::Params params = ROS2::Utils::xacro::GetParameterFromXacroData(xacroParams);
        EXPECT_EQ(params.size(), 1);
        ASSERT_TRUE(params.contains("laser_enabled"));
        EXPECT_EQ(params["laser_enabled"], "false");
    }

} // namespace UnitTest
