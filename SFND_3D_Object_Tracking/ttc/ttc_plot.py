import matplotlib.pyplot as plt
import numpy as np

def read_txt(txt_name):
    temp_list = []
    try:
        with open(txt_name) as f:
            for line in f:
                line_str = line.rstrip("\n")
                if line_str in ["nan", "-inf", "inf"]:
                    temp_list.append(-100)
                else:
                    temp_list.append(float(line_str))

    except FileNotFoundError:
        print("No " + txt_name)
    return temp_list

def report_image():
    detectors = ['SHITOMASI', 'HARRIS', 'FAST', 'BRISK', 'ORB', 'AKAZE', 'SIFT']
    descriptors = ['BRISK', 'BRIEF', 'ORB', 'FREAK', 'AKAZE', 'SIFT']
    for detector in detectors:
        fig, ax = plt.subplots()
        ax.set(xlabel='image frame index', ylabel='TTC (s)',
               title=detector, )
        ax.set_ylim(0, 40)
        descriptor_list = []
        for descriptor in descriptors:
            txt_name = detector + "_" + descriptor + ".txt"
            ttcs = read_txt(txt_name)

            if ttcs:
                # Data for plotting
                descriptor_list.append(descriptor)
                t = np.arange(1, 19, dtype=int)
                ax.plot(t, ttcs, '*')
                ax.grid()

            ax.legend(descriptor_list)
        fig.savefig("../images/report_images/" + detector + ".png")






if __name__ == '__main__':
    report_image()

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
