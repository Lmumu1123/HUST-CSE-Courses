import mysql.connector
from mysql.connector import Error



def main():
    # 连接到 MySQL 数据库
    p_user = input("请输入用户名：")
    p_password = input("请输入密码：")
    p_database = input("请输入数据库名称：")

    try:
        cnx = mysql.connector.connect(
            user=p_user,
            password=p_password,
            host='127.0.0.1',
            database=p_database
        )
        cursor = cnx.cursor()

        # 1 新生入学信息增加，学生信息修改

        def add_student(sno, sname, ssex, sage, sdept, scholarship):
            query = """
                  INSERT INTO student (sno, sname, ssex, sage, sdept, scholarship)
                  VALUES (%s, %s, %s, %s, %s, %s)
                  ON DUPLICATE KEY UPDATE 
                  sname=%s, ssex=%s, sage=%s, sdept=%s, scholarship=%s
                  """
            # 在这里捕获可能的异常并处理，比如插入重复的学号
            try:
                cursor.execute(query,
                               (sno, sname, ssex, sage, sdept, scholarship, sname, ssex, sage, sdept, scholarship))
                cnx.commit()
                print("学生信息已添加或更新")
            except mysql.connector.Error as err:
                print(f"发生错误: {err}")
                cnx.rollback()  # 发生错误时回滚

            # 2 课程信息维护：增加新课程，修改课程信息，删除没有选课的课程信息

        def add_course(cursor, cnx):
            cno = input("请输入课程号: ")
            cname = input("请输入课程名称: ")
            cpno = input("请输入先修课程号（如果无先修课程，请输入NULL）: ")
            ccredit = input("请输入课程学分: ")

            cpno = None if cpno.lower() == 'null' else cpno  # 将用户输入的 'NULL' 转化为 None
            query = "INSERT INTO course (Cno, Cname, Cpno, Ccredit) VALUES (%s, %s, %s, %s);"
            try:
                cursor.execute(query, (cno, cname, cpno, ccredit))
                cnx.commit()
                print("新增课程成功。")
            except Error as err:
                print(f"新增课程失败: {err}")
                cnx.rollback()

        def update_course(cursor, cnx):
            cno = input("请输入要修改的课程号: ")
            cname = input("请输入新的课程名称: ")
            cpno = input("请输入新的先修课程号（如果无先修课程，请输入NULL）: ")
            ccredit = input("请输入新的课程学分: ")

            cpno = None if cpno.lower() == 'null' else cpno
            query = "UPDATE course SET Cname=%s, Cpno=%s, Ccredit=%s WHERE Cno=%s;"
            try:
                cursor.execute(query, (cname, cpno, ccredit, cno))
                cnx.commit()
                print("课程信息更新成功。")
            except Error as err:
                print(f"课程信息更新失败: {err}")
                cnx.rollback()

        def delete_unenrolled_courses(cursor, cnx):
            try:
                # 更新那些已经作为先修课程的记录
                update_query = """
                    SET Foreign_key_checks = 0
                    """
                cursor.execute(update_query)

                # 删除那些没有学生选课记录的课程
                delete_query = """
                    delete from course where cno NOT IN(select cno from sc)
                    """
                cursor.execute(delete_query)

                cnx.commit()
                print("未选修的课程已删除，并更新了相关的先修课程信息。")
            except Error as err:
                cnx.rollback()
                print(f"在尝试更新或删除课程时出现错误: {err}")

            # 3 录入学生成绩，修改学生成绩

        def enter_grade(cursor, cnx):
            sno = input("请输入学生学号: ")
            cno = input("请输入课程号: ")
            grade = input("请输入学生成绩: ")

            query = "INSERT INTO sc (Sno, Cno, Grade) VALUES (%s, %s, %s) ON DUPLICATE KEY UPDATE Grade=%s;"
            try:
                cursor.execute(query, (sno, cno, grade, grade))
                cnx.commit()
                print("学生成绩录入成功。")
            except Error as err:
                print(f"学生成绩录入失败: {err}")
                cnx.rollback()

        def update_grade(cursor, cnx):
            sno = input("请输入学生学号: ")
            cno = input("请输入课程号: ")
            grade = input("请输入新的学生成绩: ")

            query = "UPDATE sc SET Grade=%s WHERE Sno=%s AND Cno=%s;"
            try:
                cursor.execute(query, (grade, sno, cno))
                cnx.commit()
                print("学生成绩更新成功。")
            except Error as err:
                print(f"学生成绩更新失败: {err}")
                cnx.rollback()

            # 4 按系统计学生的平均成绩，最好成绩，最差成绩，优秀率，不及格人数

        def calculate_department_statistics(cursor):
            # 查询并计算每个系的成绩统计信息
            query = """
            SELECT
              student.Sdept,
              AVG(sc.Grade) AS average_grade,
              MAX(sc.Grade) AS max_grade,
              MIN(sc.Grade) AS min_grade,
              SUM(CASE WHEN sc.Grade >= 90 THEN 1 ELSE 0 END) / COUNT(sc.Grade) AS excellent_rate,
              SUM(CASE WHEN sc.Grade < 60 THEN 1 ELSE 0 END) AS fail_count
            FROM student
            INNER JOIN sc ON student.Sno = sc.Sno
            GROUP BY student.Sdept;
            """

            try:
                cursor.execute(query)
                department_results = cursor.fetchall()

                print("系别 | 平均成绩 | 最高成绩 | 最低成绩 | 优秀率 | 不及格人数")
                for dept_info in department_results:
                    sdept, average_grade, max_grade, min_grade, excellent_rate, fail_count = dept_info
                    print(
                        f"{sdept} | {average_grade:.2f} | {max_grade} | {min_grade} | {excellent_rate:.2%} | {fail_count}")

            except Error as err:
                print(f"计算系别成绩统计信息失败: {err}")

            # 5 按系对学生成绩进行排名，同时显示出学生、课程和成绩信息

        def rank_students_by_department(cursor, cnx):
            # 查询并显示分系别学生成绩排名信息
            query = """
                SELECT student.Sdept, student.Sno, student.Sname, sc.Cno, sc.Grade
                FROM student
                INNER JOIN sc ON student.Sno = sc.Sno
                ORDER BY student.Sdept, sc.Grade DESC;
                """
            try:
                cursor.execute(query)
                results = cursor.fetchall()

                current_dept = ""  # 用于跟踪当前的系别名称
                for record in results:
                    dept, sno, sname, cno, grade = record

                    # 检查当前系别是否与前一个系别不同，如果是则打印系别名称
                    if dept != current_dept:
                        print("\n" + dept + ":")  # 系别名称前后各有换行符，以便间隔
                        current_dept = dept  # 更新当前系别名称

                    # 在系别名称后打印学生成绩信息
                    print(f"{sno} | {sname} | {cno} | {grade}")

            except Error as err:
                print(f"查询学生成绩排名失败: {err}")

            # 6 输入学号，显示该学生的基本信息和选课信息

        def show_student_info(cursor, cnx, student_id):
            # 查询并显示学生基本信息
            query_student_info = """
                  SELECT * FROM student
                  WHERE Sno = %s;
                  """

            # 查询并显示学生的选课信息
            query_student_courses = """
                  SELECT sc.Cno, course.Cname, sc.Grade
                  FROM sc
                  INNER JOIN course ON course.Cno = sc.Cno
                  WHERE sc.Sno = %s;
                  """
            try:
                # 执行查询学生基本信息的查询
                cursor.execute(query_student_info, (student_id,))
                student_info = cursor.fetchone()

                if student_info:
                    # 可能您需要根据表格结构调整下面输出信息的具体列明
                    print("学生基本信息:")
                    print(
                        f"学号: {student_info[0]}, 姓名: {student_info[1]}, 性别: {student_info[2]}, "
                        f"年龄: {student_info[3]}, 系别: {student_info[4]}, 奖学金: {student_info[5]}")

                    # 执行查询学生选课信息的查询
                    cursor.execute(query_student_courses, (student_id,))
                    courses_info = cursor.fetchall()

                    print("\n学生选课信息:")
                    print("课程号 | 课程名称 | 成绩")
                    for course in courses_info:
                        print(f"{course[0]} | {course[1]} | {course[2]}")
                else:
                    print("未找到该学号的学生信息。")

            except Error as err:
                print(f"查询学生信息失败: {err}")
        print("数据库连接成功！")

        while True:
            print("\n")
            print("+--------------------------------+")
            print("|  请选择要执行的功能序号：           |")
            print("|  1: 新生入学信息增加，学生信息修改   |")
            print("|  2: 课程信息维护                 |")
            print("|  3: 录入学生成绩，修改学生成绩      |")
            print("|  4: 按系计算学生平均成绩，成绩分布   |")
            print("|  5: 按系排名学生成绩              |")
            print("|  6: 显示学生基本信息和选课信息      |")
            print("|  0: 退出程序                    |")
            print("+--------------------------------+")
            choice = input("请输入功能序号：")

            if choice == '1':
                # 新生入学信息增加，学生信息修改相关代码
                try:
                    # 获取用户输入
                    sno = input("请输入学号: ")
                    sname = input("请输入姓名: ")
                    ssex = input("请输入性别: ")
                    sage = input("请输入年龄: ")
                    sdept = input("请输入系别: ")
                    scholarship = input("请输入奖学金情况(是/否): ")
                    # 调用函数
                    add_student(sno, sname, ssex, sage, sdept, scholarship)
                except Exception as e:
                    print(f"无法添加学生信息: {e}")

            elif choice == '2':
                    while True:
                        print("\n功能菜单：")
                        print("1 - 增加新课程")
                        print("2 - 修改课程信息")
                        print("3 - 删除没有选课的课程信息")
                        print("0 - 返回上级菜单")
                        choice = input("请输入您的选择: ")

                        if choice == '1':
                            add_course(cursor, cnx)
                        elif choice == '2':
                            update_course(cursor, cnx)
                        elif choice == '3':
                            delete_unenrolled_courses(cursor, cnx)
                        elif choice == '0':
                            break
                        else:
                            print("无效的选项，请重新输入。")

            elif choice == '3':
                        print("\n功能菜单：")
                        print("1 - 录入学生成绩")
                        print("2 - 修改学生成绩")
                        print("0 - 返回上级菜单")
                        choice = input("请输入您的选择: ")

                        if choice == '1':
                            enter_grade(cursor, cnx)
                        elif choice == '2':
                            update_grade(cursor, cnx)
                        elif choice == '0':
                            break
                        else:
                            print("无效的选项，请重新输入。")

            elif choice == '4':
                    print("\n按系计算并输出学生成绩的平均值、最高分、最低分、优秀率和不及格人数。")
                    # 调用函数计算每个系的统计信息
                    calculate_department_statistics(cursor)

            elif choice == '5':
                    print("\n按系对学生成绩进行排名，同时显示出学生、课程和成绩信息：")
                    rank_students_by_department(cursor, cnx)

            elif choice == '6':
                    student_id = input("\n请输入要查询的学生学号：")
                    show_student_info(cursor, cnx, student_id)

            elif choice == '0':
                print("退出程序。")
                break
            else:
                print("输入无效，请输入一个有效序号。")

    except Error as e:
        print(f"Error while connecting to MySQL: {e}")
    finally:
        # 确保即便出现错误也能正确关闭游标和连接
        if cnx.is_connected():
            cursor.close()
            cnx.close()
            print("数据库连接已关闭。")

# 程序入口
if __name__ == "__main__":
    main()
