// #include "Editor.hpp"
#include "Function/General/KDTree.hpp"
#include "Function/General/Utils.hpp"

using namespace aEngine;

template <std::size_t Dim>
std::vector<std::array<float, Dim>> generateData(int numPoints) {
  // Initialize random number generator
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0.0, 100.0); // Range [0.0, 100.0]

  std::vector<std::array<float, Dim>> data;
  data.reserve(numPoints);

  for (int i = 0; i < numPoints; ++i) {
    std::array<float, Dim> point;
    for (int j = 0; j < Dim; ++j)
      point[j] = static_cast<float>(dis(gen));
    data.emplace_back(point);
  }
  return data;
}

int main() {
  // Editor editor(1920, 1080);
  // editor.Start();

  // // the main loop
  // editor.Run(false);

  // editor.Shutdown();

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow *window = glfwCreateWindow(1920, 1080, "Engine", NULL, NULL);
  if (!window) {
    LOG_F(ERROR, "Failed to create GLFW window");
    return -1;
  }
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    LOG_F(ERROR, "Failed to load GLAD");
    return -1;
  }

  const int dataDim = 24;
  int dataSize = 4e4;
  auto data = generateData<dataDim>(dataSize);

  KDTree<float, dataDim> tree;

  tree.Build(data);

  const int testSize = 1, range = 100;
  std::array<float, dataDim> bbmin, bbmax;
  bbmin.fill(std::numeric_limits<float>::max());
  bbmax.fill(-std::numeric_limits<float>::max());
  for (auto &a : data) {
    for (int i = 0; i < dataDim; ++i) {
      bbmin[i] = std::min(bbmin[i], a[i]);
      bbmax[i] = std::max(bbmax[i], a[i]);
    }
  }
  Timer timer;
  std::vector<std::array<float, dataDim>> testSamples;
  for (int i = 0; i < testSize; ++i) {
    std::array<float, dataDim> p;
    float x = (float)RandDouble();
    float y = (float)RandDouble();
    float z = (float)RandDouble();
    for (int j = 0; j < dataDim; ++j)
      p[j] = bbmin[j] + (float)RandDouble() * (bbmax[j] - bbmin[j]);
    testSamples.push_back(p);
  }

  std::vector<int> j1, j2, j3;
  printf("test start\n");
  timer.Reset();
  for (int i = 0; i < testSize; ++i)
    j1.push_back(tree.NearestSearch(testSamples[i]));
  auto t1 = timer.ElapsedMilliseconds();
  printf("kdtree search %lf ms\n", t1);
  timer.Reset();
  for (int i = 0; i < testSize; ++i)
    j2.push_back(tree.BruteForceNearestSearch(testSamples[i]));
  auto t2 = timer.ElapsedMilliseconds();
  printf("brute force %lf ms\n", t2);
  timer.Reset();
  for (int i = 0; i < testSize; ++i)
    j3.push_back(tree.ParelleNearestSearch(testSamples[i]));
  auto t3 = timer.ElapsedMilliseconds();
  printf("gpu search %lf ms\n", t3);

  for (int i = 0; i < testSize; ++i) {
    if (j3[i] != j2[i])
      printf("error for sample %d, x=%f, y=%f, z=%f\n", i, testSamples[i][0],
             testSamples[i][1], testSamples[i][2]);
  }

  return 0;
}