import matplotlib.pyplot as plt

def plot_sa_curve(filename):
    temperatures = []
    best_costs = []

    # 读取文件并解析数据
    with open(filename, 'r') as file:
        for line in file:
            if line.startswith('Temperature:'):
                # 解析温度和bestCost
                parts = line.split(',')
                temp = float(parts[0].split(':')[1].strip())
                area = float(parts[1].split(':')[1].strip())
                wire_length = float(parts[2].split(':')[1].strip())
                best_cost = float(parts[3].split(':')[1].strip())
                
                temperatures.append(temp)
                best_costs.append(best_cost)

    # 绘制曲线图，保持数据原始顺序
    plt.plot(temperatures, best_costs, marker='o', linestyle='-', color='b')
    plt.xlabel('Temperature')
    plt.ylabel('Best Cost')
    plt.title('Simulated Annealing Best Cost vs Temperature')
    plt.grid(True)
    plt.gca().invert_xaxis()  # 反转 x 轴刻度，使温度从高到低排列
    plt.tight_layout()

    # 保存图形为文件
    plt.savefig('sa_curve.png')
    plt.close()  # 关闭图形，释放资源

if __name__ == '__main__':
    plot_sa_curve('/root/workspace/YuFloorPlan/tmp.log')
