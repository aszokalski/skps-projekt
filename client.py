import socket
import math
from matplotlib import pyplot as plt
from matplotlib.ticker import FormatStrFormatter

HOST = "10.42.0.200"
PORT = 8080

MAX_DEG = 180
MAX_DISTANCE = 350


def get_plot_coordinates(degree, distance):
    x = distance * math.sin(math.radians(degree))
    y = distance * math.cos(math.radians(degree))
    return x, y


def prepare_plot():
    plt.ion()
    fig, ax = plt.subplots(figsize=(8, 8))
    plt.title("lidar-servo")
    plt.xlabel("distance [mm]")
    plt.ylabel("distance [mm]")
    plt.gca().set_aspect('equal')

    ax.set_xlim(-10, MAX_DISTANCE)
    ax.set_ylim(-MAX_DISTANCE, MAX_DISTANCE)
    plt.show()

    return fig, ax


def get_line_color(distance):
    if distance < 50:
        return 'red'
    elif distance < 100:
        return 'yellow'
    else:
        return 'green'


def update_plot(fig, lines, degree, distance):
    x, y = get_plot_coordinates(degree, distance)
    lines[degree].set_data([0, x], [0, y])
    lines[degree].set_color(get_line_color(distance))

    fig.canvas.draw_idle()
    fig.canvas.flush_events()


def main():
    fig, ax = prepare_plot()
    lines = [ax.plot([0, 0], [0, 0], '-o')[0] for _ in range(MAX_DEG + 1)]

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))
        s.sendall(b"Hello, world")

        while True:
            measurement_data = s.recv(256).decode("utf-8").split(",")
            degree = int(measurement_data[0])
            distance = int(measurement_data[1])

            update_plot(fig, lines, degree, distance)


if __name__ == "__main__":
    main()
