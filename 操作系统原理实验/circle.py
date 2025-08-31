import pygame
import threading
import math
import time

# 初始化pygame
pygame.init()

# 定义常量
WIDTH, HEIGHT = 480, 360
PI = math.pi
YELLOW = (255, 255, 0)
GREEN = (0, 255, 0)

# 设置窗口
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("画正方形与圆")

# 画正方形的线程函数
def draw_square():
    for i in range(180):
        screen.set_at((50 + i, 50), YELLOW)  # 画第一条边
        pygame.display.update()
        time.sleep(0.005)

    for i in range(180):
        screen.set_at((50 + 180, 50 + i), YELLOW)  # 画第二条边
        pygame.display.update()
        time.sleep(0.005)

    for i in range(180):
        screen.set_at((50 + 180 - i, 50 + 180), YELLOW)  # 画第三条边
        pygame.display.update()
        time.sleep(0.005)

    for i in range(180):
        screen.set_at((50, 50 + 180 - i), YELLOW)  # 画第四条边
        pygame.display.update()
        time.sleep(0.005)

# 画圆的线程函数
def draw_circle():
    for i in range(720):
        x = int(350 + 100 * math.cos(-PI / 2 + (i * PI) / 360))
        y = int(140 + 100 * math.sin(-PI / 2 + (i * PI) / 360))
        screen.set_at((x, y), GREEN)
        pygame.display.update()
        time.sleep(0.005)

# 创建并启动线程
def main():
    # 创建线程
    square_thread = threading.Thread(target=draw_square)
    circle_thread = threading.Thread(target=draw_circle)

    square_thread.start()
    circle_thread.start()

    # 等待所有线程结束
    square_thread.join()
    circle_thread.join()

    # 保持窗口直到用户关闭
    pygame.time.wait(2000)  # 等待2秒钟关闭窗口
    pygame.quit()

if __name__ == "__main__":
    main()
