#include "converter_general.h"
#include "MitsubaGenerator.h"

using namespace std;
using namespace chrono;
using namespace chrono::collision;

int main(int argc, char* argv[]) {
  if (argc == 1) {
    cout << "REQURES FRAME NUMBER AS ARGUMENT, ONLY CREATING SCENE" << endl;
    MitsubaGenerator scene_document("scene.xml");
    scene_document.camera_origin = ChVector<>(.2, .65, -.35) * 100;
    scene_document.camera_target = ChVector<>(-.4, .4, .4) * 100;
    scene_document.scale = 3;
    scene_document.turbidity = 10;
    scene_document.CreateScene();
    scene_document.AddShape("background", ChVector<>(1), ChVector<>(0),
                            ChQuaternion<>(1, 0, 0, 0));
    scene_document.Write();
    return 0;
  }
  stringstream input_file_ss;
  input_file_ss << argv[1] << ".txt";

  string data;
  ReadCompressed(input_file_ss.str(), data);
  std::replace(data.begin(), data.end(), ',', '\t');
  stringstream output_file_ss;
  if (argc == 3) {
    output_file_ss << argv[2] << argv[1] << ".xml";
  } else {
    output_file_ss << argv[1] << ".xml";
  }

  MitsubaGenerator data_document(output_file_ss.str());
  stringstream data_stream(data);
  ChQuaternion<> rot;
  ChVector<> pos, vel, scale;
  int counter = 0;

  // SkipLine(data_stream, 7);

  // SkipLine(data_stream, 4);

  ProcessPovrayLine(data_stream, pos, vel, scale, rot);
  scale.z = scale.x;
  data_document.AddShape("cylinder_oth", scale, pos, rot);
  //  ProcessPovrayLine(data_stream, pos, vel, scale, rot);
  //  data_document.AddShape("flat", scale, pos, rot);
  //  ProcessPovrayLine(data_stream, pos, vel, scale, rot);
  //  data_document.AddShape("flat", scale, pos, rot);
  ProcessPovrayLine(data_stream, pos, vel, scale, rot);
  scale.z = scale.x;
  data_document.AddShape("cylinder_oth", scale, pos, rot);
  SkipLine(data_stream, 6);  // 18
  ProcessPovrayLine(data_stream, pos, vel, scale, rot);
  data_document.AddShape("flat", scale, pos, rot);

  while (data_stream.fail() == false) {
    int type = ProcessPovrayLine(data_stream, pos, vel, scale, rot);
    if (data_stream.fail() == false) {
      switch (type) {
        case chrono::collision::SPHERE:
          scale.y = scale.z = scale.x;
          data_document.AddShape("sphere", scale, pos, rot);
          break;
        case chrono::collision::ELLIPSOID:
          data_document.AddShape("ellipsoid", scale, pos, rot);
          break;
        case chrono::collision::BOX:
          data_document.AddShape("cube", scale, pos, rot);
          break;
        case chrono::collision::CYLINDER:
          scale.z = scale.x;
          data_document.AddShape("cylinder", scale, pos, rot);
          break;
      }
    }
  }

  data_document.Write();
  return 0;
}
