#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <cstdlib>
#include <ctime>

class Module {
public:
    Module() : width(0), height(0), x(0), y(0) {} 
    Module(int width, int height) : width(width), height(height), x(0), y(0) {}

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getX() const { return x; }
    int getY() const { return y; }

    void setX(int newX) { x = newX; }
    void setY(int newY) { y = newY; }

private:
    int width;
    int height;
    int x;
    int y;
};

class ModuleLayout {
public:
    ModuleLayout() {}

    void updateModules(const std::vector<Module> &newModules) {
        modules = newModules;
    }

    void readModules(const std::string &filename) {
        std::ifstream file(filename);
        if (!file) {
            throw std::runtime_error("Unable to open module file");
        }

        int n;
        file >> n;
        modules.resize(n);
        for (int i = 0; i < n; ++i) {
            int width, height;
            file >> width >> height;
            modules[i] = Module(width, height);
        }
        file.close();
    }

    void readConnections(const std::string &filename) {
        std::ifstream file(filename);
        if (!file) {
            throw std::runtime_error("Unable to open connection file");
        }

        int n;
        file >> n;
        connections.resize(n);
        std::string line;
        std::getline(file, line); 
        for (int i = 0; i < n; ++i) {
            std::getline(file, line);
            std::istringstream iss(line);
            int x;
            while (iss >> x) {
                connections[i].push_back(x);
            }
        }
        file.close();
    }

    void maxDensityLayout() {
        std::sort(modules.begin(), modules.end(), [](const Module &a, const Module &b) {
            return (a.getWidth() * a.getHeight()) > (b.getWidth() * b.getHeight());
        });

        int currentX = 0, currentY = 0, maxHeight = 0;

        for (auto &module : modules) {
            module.setX(currentX);
            module.setY(currentY);
            currentX += module.getWidth();
            maxHeight = std::max(maxHeight, module.getHeight());

            if (currentX > 100) {  
                currentX = 0;
                currentY += maxHeight;
                maxHeight = 0;
            }
        }
    }

    double calculateTotalArea() const {
        int maxX = 0, maxY = 0;
        for (const auto &module : modules) {
            maxX = std::max(maxX, module.getX() + module.getWidth());
            maxY = std::max(maxY, module.getY() + module.getHeight());
        }
        return static_cast<double>(maxX) * maxY;
    }

    double calculateTotalWireLength() const {
        double totalWireLength = 0.0;
        for (const auto &conn : connections) {
            if (conn.size() < 2) continue;
            for (size_t i = 1; i < conn.size(); ++i) {
                int idx1 = conn[0] - 1;
                int idx2 = conn[i] - 1;
                int x1 = modules[idx1].getX() + modules[idx1].getWidth() / 2;
                int y1 = modules[idx1].getY() + modules[idx1].getHeight() / 2;
                int x2 = modules[idx2].getX() + modules[idx2].getWidth() / 2;
                int y2 = modules[idx2].getY() + modules[idx2].getHeight() / 2;
                totalWireLength += std::abs(x1 - x2) + std::abs(y1 - y2);
            }
        }
        return totalWireLength;
    }

    const std::vector<Module>& getModules() const { return modules; }
    const std::vector<std::vector<int>>& getConnections() const { return connections; }

private:
    std::vector<Module> modules;
    std::vector<std::vector<int>> connections;
};

class SimulatedAnnealing {
public:
    SimulatedAnnealing(ModuleLayout &layout) : layout(layout), bestModules(layout.getModules()), bestCost(0) {}

    // void recordCost(const std::string &filename, double temperature, double cost) const { // test
    //     std::ofstream file(filename, std::ios::app); // 打开文件进行追加写入
    //     if (!file) {
    //         std::cerr << "Error: Unable to open file " << filename << std::endl;
    //         return;
    //     }
    //     file << temperature << " " << cost << std::endl;
    //     file.close();
    // }

    void optimize() {
    double T = 1000.0;
    double T_min = 0.1;
    double alpha = 0.9;
    double alpha_ir = 1.05;

    bestCost = calculateCost(layout.getModules(), layout.getConnections());

    // std::ofstream costFile("cost_vs_temperature.txt"); // 创建用于记录 cost 的文件

    while (T > T_min) {
        for (int i = 0; i < 100; ++i) {
            std::vector<Module> newModules = layout.getModules();
            perturbLayout(newModules);
            double newCost = calculateCost(newModules, layout.getConnections());
            double delta = newCost - bestCost;
            if (delta < 0 || (exp(-delta / T) > ((double)rand() / RAND_MAX))) {
                layout.updateModules(newModules);
                if (newCost < bestCost) {
                    bestModules = newModules;
                    bestCost = newCost;
                }
            }
        }

        double currentArea = layout.calculateTotalArea();
        double currentWireLength = layout.calculateTotalWireLength();
        std::cout << "Temperature: " << T << ", Area: " << currentArea << ", Wire Length: " 
                  << currentWireLength << ", bestCost: " << bestCost << std::endl;

        // recordCost("cost_vs_temperature.txt", T, bestCost); // 记录当前温度和最佳 cost 到文件

        alpha = (0.95 < alpha_ir * alpha) ? 0.95 : alpha_ir * alpha;
        T *= alpha;
    }

    // costFile.close();

    layout.updateModules(bestModules); // 最终将最佳模块布局更新到 layout
}

private:
    ModuleLayout &layout;
    std::vector<Module> bestModules;
    double bestCost;

    double calculateCost(const std::vector<Module> &modules, const std::vector<std::vector<int>> &connections) const {
        double weight_area = 0.04;
        double weight_wireLenght = 0.06;
        double area = layout.calculateTotalArea();
        double wireLength = layout.calculateTotalWireLength();
        return weight_area * area + weight_wireLenght * wireLength;
    }

    void perturbLayout(std::vector<Module> &modules) {
        int idx1 = rand() % modules.size();
        int idx2 = rand() % modules.size();

        if (rand() % 2 == 0) {
            std::swap(modules[idx1], modules[idx2]);
        } else {
            modules[idx1].setX(modules[idx1].getX() + rand() % 31 - 15);
            modules[idx1].setY(modules[idx1].getY() + rand() % 31 - 15);
            modules[idx2].setX(modules[idx2].getX() + rand() % 31 - 15);
            modules[idx2].setY(modules[idx2].getY() + rand() % 31 - 15);

            modules[idx1].setX(std::max(modules[idx1].getX(), 0));
            modules[idx1].setY(std::max(modules[idx1].getY(), 0));
            modules[idx2].setX(std::max(modules[idx2].getX(), 0));
            modules[idx2].setY(std::max(modules[idx2].getY(), 0));
        }
    }
};

int main(int argc, char *argv[]) {
    srand(time(0));

    try {
        ModuleLayout layout;
        std::string module_file_name = argv[1];
        std::string module_connect_file_name = argv[2];
        // std::cout<<"Reading file "<<module_file_name<<std::endl;
        // std::cout<<"Reading file "<<module_connect_file_name<<std::endl;
        layout.readModules(module_file_name);
        layout.readConnections(module_connect_file_name);

        layout.maxDensityLayout();  

        SimulatedAnnealing sa(layout);
        sa.optimize();

        std::cout << "Final Total Area: " << layout.calculateTotalArea() << std::endl;
        std::cout << "Final Total Wire Length: " << layout.calculateTotalWireLength() << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
