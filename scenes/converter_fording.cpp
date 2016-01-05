#include "converter_general.h"
#include "MitsubaGenerator.h"

using namespace std;
using namespace chrono;

double vehicle_speed, driveshaft_speed, motor_torque, motor_speed, output_torque;
double wheel_torque_0, wheel_torque_1, wheel_torque_2, wheel_torque_3;
double wheel_linvel_0, wheel_linvel_1, wheel_linvel_2, wheel_linvel_3;
double wheel_angvel_0, wheel_angvel_1, wheel_angvel_2, wheel_angvel_3;
double spring_def_fl, spring_def_fr, spring_def_rl, spring_def_rr;
double shock_len_fl, shock_len_fr, shock_len_rl, shock_len_rr;
std::vector<std::tuple<int, int, std::string> > labels;

void ReadStats(std::string filename) {
    ifstream ifile(filename.c_str());

    string line;
    getline(ifile, line);
    std::replace(line.begin(), line.end(), ',', '\t');
    stringstream ss(line);
    ss >> vehicle_speed >> driveshaft_speed >> motor_torque >> motor_speed >> output_torque;
    ss >> wheel_torque_0 >> wheel_torque_1 >> wheel_torque_2 >> wheel_torque_3;
    ss >> wheel_angvel_0 >> wheel_angvel_1 >> wheel_angvel_2 >> wheel_angvel_3;
    ss >> spring_def_fl >> spring_def_fr >> spring_def_rl >> spring_def_rr;
    ss >> shock_len_fl >> shock_len_fr >> shock_len_rl >> shock_len_rr;
    ifile.close();

    labels.push_back(std::make_tuple(
        25, 25, "vehicle speed: " + std::to_string(vehicle_speed) + " [m/s] driveshaft speed: " + std::to_string(driveshaft_speed) + " [rad/s]"));
    labels.push_back(std::make_tuple(25, 50, "motor torque: " + std::to_string(motor_torque) + " [Nm] motor speed: " + std::to_string(motor_speed) +
                                                 " [rad/s] output torque: " + std::to_string(output_torque) + "[Nm]"));
    labels.push_back(std::make_tuple(25, 75, "wheel torques: [" + std::to_string(wheel_torque_0) + ", " + std::to_string(wheel_torque_1) + ", " +
                                                 std::to_string(wheel_torque_2) + ", " + std::to_string(wheel_torque_3) + "] [Nm]"));
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        cout << "REQURES FRAME NUMBER AS ARGUMENT, ONLY CREATING SCENE" << endl;
        MitsubaGenerator scene_document("scene.xml");
        scene_document.camera_origin = ChVector<>(0, -10, 0);
        scene_document.camera_target = ChVector<>(0, 0, 0);
        scene_document.camera_up = ChVector<>(0, 0, 1);
        scene_document.scale = 3;
        scene_document.turbidity = 10;
        scene_document.CreateScene();
        // scene_document.AddShape("background", ChVector<>(1), ChVector<>(0, -10,
        // 0), ChQuaternion<>(1, 0, 0, 0));
        scene_document.Write();
        return 0;
    }
    stringstream input_file_ss, input_file_vehicle, input_stats_vehicle;
    input_file_ss << "data_" << argv[1] << ".dat";
    input_file_vehicle << "vehicle_" << argv[1] << ".dat";
    input_stats_vehicle << "stats_" << argv[1] << ".dat";

    std::cout << "Opening Files: " << input_file_ss.str() << " " << input_file_vehicle.str() << " " << input_stats_vehicle.str() << "\n";
    std::cout << "Stats\n";
    ReadStats(input_stats_vehicle.str());

    std::ifstream ifile;
    std::cout << "OpenBinary\n";
    OpenBinary(input_file_ss.str(), ifile);

    std::vector<real3> position;
    std::vector<real3> velocity;
    std::cout << "ReadBinary\n";
    ReadBinary(ifile, position);
    ReadBinary(ifile, velocity);
    std::cout << "CloseBinary\n";
    CloseBinary(ifile);

    string data_v;
    std::cout << "ReadCompressed\n";
    ReadCompressed(input_file_vehicle.str(), data_v);
    std::replace(data_v.begin(), data_v.end(), ',', '\t');
    stringstream output_file_ss;
    if (argc == 3) {
        output_file_ss << argv[2] << argv[1] << ".xml";
    } else {
        output_file_ss << argv[1] << ".xml";
    }
    MitsubaGenerator data_document(output_file_ss.str());

    stringstream vehicle_stream(data_v);

    ChVector<> pos, vel, scale;
    ChQuaternion<> rot;
    int count = 0;

    for (int i = 0; i < position.size(); i++) {
        pos.x = position[i].x;
        pos.y = position[i].y;
        pos.z = position[i].z;
//        vel.x = velocity[i].x;
//        vel.y = velocity[i].y;
//        vel.z = velocity[i].z;
//        double v = vel.Length() * 2;

         data_document.AddShape("sphere", .016, pos, QUNIT);
        //data_document.AddCompleteShape("sphere", "diffuse", VelToColor(v), .016, pos, QUNIT);

        count++;
    }

    // std::cout << data_v << std::endl;

    ProcessPovrayLine(vehicle_stream, pos, vel, scale, rot);
    data_document.AddShape("box", scale, pos, rot);
    SkipLine(vehicle_stream, 3);
    //    ProcessPovrayLine(vehicle_stream, pos, vel, scale, rot);
    //    data_document.AddShape("box", scale, pos, rot);
    //    ProcessPovrayLine(data_stream, pos, vel, scale, rot);
    //    data_document.AddShape("box", scale, pos, rot);
    ProcessPovrayLine(vehicle_stream, pos, vel, scale, rot);
    data_document.AddShape("box", scale, pos, rot);
    SkipLine(vehicle_stream, 1);
    ProcessPovrayLine(vehicle_stream, pos, vel, scale, rot);
    data_document.AddShape("box", scale, pos, rot);
    SkipLine(vehicle_stream, 1);
    ProcessPovrayLine(vehicle_stream, pos, vel, scale, rot);
    data_document.AddShape("box", scale, pos, rot);
    ProcessPovrayLine(vehicle_stream, pos, vel, scale, rot);
    data_document.AddShape("box", scale, pos, rot);

    ProcessPovrayLine(vehicle_stream, pos, vel, scale, rot);
    // chrono::ChQuaternion<> q;
    // q.Q_from_AngAxis(CH_C_PI, chrono::ChVector<>(1, 0, 0));
    /// rot = rot * q;
    Vector offset = Vector(0, 0, 0);  // rot.Rotate(Vector(-0.055765, 0, -0.52349));
    data_document.AddShape("chassis", Vector(1, 1, 1), pos + offset, rot);

    Vector camera_pos = pos + offset;
    camera_pos.z = 4;
    camera_pos.y -= 8;
    camera_pos.x += 0;
    data_document.AddSensor(camera_pos, pos + offset, Vector(0, 0, 1), labels);

    SkipLine(vehicle_stream, 5);
    ProcessPovrayLine(vehicle_stream, pos, vel, scale, rot);
    data_document.AddShape("wheel_R", Vector(1, 1, 1), pos, rot);
    SkipLine(vehicle_stream, 22);
    ProcessPovrayLine(vehicle_stream, pos, vel, scale, rot);
    data_document.AddShape("wheel_R", Vector(1, -1, 1), pos, rot);
    SkipLine(vehicle_stream, 22);
    ProcessPovrayLine(vehicle_stream, pos, vel, scale, rot);
    data_document.AddShape("wheel_R", Vector(1, 1, 1), pos, rot);
    SkipLine(vehicle_stream, 22);
    ProcessPovrayLine(vehicle_stream, pos, vel, scale, rot);
    data_document.AddShape("wheel_R", Vector(1, -1, 1), pos, rot);

    data_document.Write();
    return 0;
}
