#include "MitsubaGenerator.h"

using namespace chrono;
using namespace std;
using namespace rapidxml;

xml_node<>* MitsubaGenerator::CreateNewNode(const std::string& type) {
  char* node_name = scene_doc.allocate_string(type.c_str());
  return scene_doc.allocate_node(node_element, node_name);
}
void MitsubaGenerator::AddAttribute(const std::string& name, const std::string& value, xml_node<>* node) {
  char* n = scene_doc.allocate_string(name.c_str());
  char* v = scene_doc.allocate_string(value.c_str());
  node->append_attribute(scene_doc.allocate_attribute(n, v));
}
void MitsubaGenerator::AddAttributes(const std::vector<xml_option>& options, xml_node<>* root) {
  for (int i = 0; i < options.size(); i++) {
    xml_node<>* node = CreateNewNode(options[i].type.c_str());
    AddAttribute("name", options[i].parameter.c_str(), node);
    AddAttribute("value", options[i].value.c_str(), node);
    root->append_node(node);
  }
}
xml_node<>* MitsubaGenerator::CreatePlugin(const std::string& name,
                                           const std::string& type,
                                           const std::vector<xml_option>& options) {
  xml_node<>* plugin_root = CreateNewNode(name.c_str());  // Create the root integrator node <integrator ...
  AddAttribute("type", type.c_str(), plugin_root);        // Set the type of integrator <integrator type="path">
  AddAttributes(options, plugin_root);                    // Set the other integrator options
  return plugin_root;                                     // Add the node to the root
                                                          // ////root_node->append_node(integrator_root);
}
rapidxml::xml_node<>* MitsubaGenerator::CreateTransform(const std::string& name) {
  xml_node<>* transform_root = CreateNewNode("transform");
  AddAttribute("name", name.c_str(), transform_root);
  return transform_root;
}

rapidxml::xml_node<>* MitsubaGenerator::CreateTransform(const ChVector<>& scale,
                                                        const ChVector<>& position,
                                                        const ChQuaternion<>& rotation) {
  xml_node<>* transform_root = CreateTransform("toWorld");

  Scale(scale, transform_root);

  double angle;
  ChVector<> axis;
  rotation.Q_to_AngAxis(angle, axis);
  angle = angle * 180.0 / CH_C_PI;
  Rotate(angle, axis, transform_root);
  Translate(position, transform_root);
  return transform_root;
}

std::string MitsubaGenerator::CreateTriplet(const ChVector<>& v) {
  stringstream ss;
  ss << v.x << ", " << v.y << ", " << v.z;
  return ss.str();
}
void MitsubaGenerator::CreateXYZTriplet(const ChVector<>& v, xml_node<>* node) {
  AddAttribute("x", std::to_string(v.x), node);
  AddAttribute("y", std::to_string(v.y), node);
  AddAttribute("z", std::to_string(v.z), node);
}

void MitsubaGenerator::Translate(const ChVector<>& vector, rapidxml::xml_node<>* root) {
  xml_node<>* node = CreateNewNode("translate");
  CreateXYZTriplet(vector, node);

  root->append_node(node);
}
void MitsubaGenerator::Scale(const ChVector<>& vector, rapidxml::xml_node<>* root) {
  xml_node<>* node = CreateNewNode("scale");
  CreateXYZTriplet(vector, node);
  root->append_node(node);
}
void MitsubaGenerator::Rotate(const double angle, const ChVector<>& axis, rapidxml::xml_node<>* root) {
  xml_node<>* node = CreateNewNode("rotate");

  CreateXYZTriplet(axis, node);
  AddAttribute("angle", std::to_string(angle), node);
  root->append_node(node);
}

void MitsubaGenerator::LookAt(const ChVector<>& origin,
                              const ChVector<>& look_at,
                              const ChVector<>& up,
                              rapidxml::xml_node<>* root) {
  xml_node<>* node = CreateNewNode("lookAt");

  AddAttribute("origin", CreateTriplet(origin).c_str(), node);
  AddAttribute("target", CreateTriplet(look_at).c_str(), node);
  AddAttribute("up", CreateTriplet(up).c_str(), node);

  root->append_node(node);
}

rapidxml::xml_node<>* MitsubaGenerator::CreateSky(const std::string& scale,
                                                  const std::string& hour,
                                                  const std::string& turbidity,
                                                  const std::string& albedo) {
  std::vector<xml_option> sky_options;
  sky_options.push_back(xml_option("float", "scale", scale));
  sky_options.push_back(xml_option("float", "hour", hour));
  sky_options.push_back(xml_option("float", "turbidity", turbidity));
  sky_options.push_back(xml_option("spectrum", "albedo", albedo));
  xml_node<>* sky = CreatePlugin("emitter", "sky", sky_options);
  return sky;
}

void MitsubaGenerator::CreateScene(bool add_integrator, bool add_sensor, bool add_sky) {
  xml_node<>* geom_include = CreateNewNode("include");
  AddAttribute("filename", "geometry.xml", geom_include);
  root_node->append_node(geom_include);

  if (add_integrator) {
    /////Integrator
    std::vector<xml_option> integrator_options;
    integrator_options.push_back(xml_option("integer", "maxDepth", "10"));
    integrator_options.push_back(xml_option("integer", "rrDepth", "10"));
    xml_node<>* integrator = CreatePlugin("integrator", "path", integrator_options);
    root_node->append_node(integrator);
  }
  if (add_sensor) {
    AddSensor(camera_origin, camera_target, camera_up);
  }
  if (add_sky) {
    // Add the Sky emitter
    root_node->append_node(CreateSky(to_string(scale), to_string(hour), to_string(turbidity), to_string(albedo)));
  }
  xml_node<>* data_include = CreateNewNode("include");
  AddAttribute("filename", "$frame.xml", data_include);
  root_node->append_node(data_include);
}

MitsubaGenerator::MitsubaGenerator() {
  // create xml header

  xml_node<>* decl = scene_doc.allocate_node(node_declaration);
  AddAttribute("version", "1.0", decl);
  AddAttribute("encoding", "utf-8", decl);
  scene_doc.append_node(decl);

  root_node = CreateNewNode("scene");
  AddAttribute("version", "0.5.0", root_node);
  scene_doc.append_node(root_node);

  width = 1920;
  height = 1080;

  camera_target = ChVector<>(0, -3, 0);
  camera_origin = ChVector<>(0, -2, 20);
  camera_up = ChVector<>(0, 1, 0);

  scale = 3;
  hour = 12;
  turbidity = 10;
  albedo = .15;
}

void MitsubaGenerator::AddShape(const std::string& id,
                                const ChVector<>& scale,
                                const ChVector<>& position,
                                const ChQuaternion<>& rotation) {
  xml_node<>* shape_node = CreateNewNode("shape");  // Create the root integrator node <integrator ...
  AddAttribute("type", "instance", shape_node);     // Set the type of integrator <integrator type="path">
  xml_node<>* reference = CreateNewNode("ref");
  AddAttribute("id", id, reference);
  shape_node->append_node(reference);
  xml_node<>* transform = CreateTransform(scale, position, rotation);
  shape_node->append_node(transform);

  root_node->append_node(shape_node);
}

void MitsubaGenerator::AddSensor(ChVector<> origin, ChVector<> target, ChVector<> up) {
  /////Sensor
  std::vector<xml_option> sensor_options;
  sensor_options.push_back(xml_option("string", "fovAxis", "smaller"));
  sensor_options.push_back(xml_option("float", "fov", "45"));
  xml_node<>* sensor = CreatePlugin("sensor", "perspective", sensor_options);
  xml_node<>* sensor_transform = CreateTransform("toWorld");
  LookAt(origin, target, up, sensor_transform);
  sensor->append_node(sensor_transform);

  /////Sampler
  std::vector<xml_option> sampler_options;
  sampler_options.push_back(xml_option("integer", "sampleCount", "256"));
  sampler_options.push_back(xml_option("integer", "scramble", "-1"));
  xml_node<>* sampler = CreatePlugin("sampler", "sobol", sampler_options);
  sensor->append_node(sampler);

  /////Film
  std::vector<xml_option> film_options;
  film_options.push_back(xml_option("string", "pixelFormat", "rgba"));
  film_options.push_back(xml_option("integer", "width", to_string(width)));
  film_options.push_back(xml_option("integer", "height", to_string(height)));
  film_options.push_back(xml_option("boolean", "banner", "false"));
  xml_node<>* film = CreatePlugin("film", "ldrfilm", film_options);
  sensor->append_node(film);
  root_node->append_node(sensor);
}

void MitsubaGenerator::Write(const std::string& filename) {
  ofstream ofile(filename);
  ofile << scene_doc;
}

void MitsubaGenerator::ExportDriver(const std::string& filename) {
}