
import difflib
from time import sleep
import zipfile
import os, sys, stat
import shutil
import subprocess

zip_name = "2024代码生成公共测试程序库"
work_path = "D:\c++_code\COMPILER\code\src\\"
src_path = work_path
zip_path = os.path.join(work_path, zip_name + '.zip')
extract_folder = os.path.join(work_path,'extracted_files')
exe_name = 'compiler.exe'
testfile_name = 'testfile.txt'
ans_name = 'ans.txt'
mid_name = 'mid.ll'
error_name = 'error.txt'
diff_name = 'diff.html'
run_path = os.path.join(work_path, 'run')
curcase_path = ''

def del_dir(file_path):
    while 1:
        if not os.path.exists(file_path):
            break
        try:
            shutil.rmtree(file_path)
        except PermissionError as e:
            err_file_path = str(e).split("\'", 2)[1]
            if os.path.exists(err_file_path):
                os.chmod(err_file_path, stat.S_IWUSR)

def build_work_place():
    if os.path.exists(run_path):
        del_dir(run_path)
    os.makedirs(run_path)
    shutil.copyfile(os.path.join(src_path, exe_name),
                    os.path.join(run_path, exe_name))
    shutil.copyfile(os.path.join(curcase_path, testfile_name), os.path.join(run_path,  testfile_name))
    shutil.copyfile(os.path.join(curcase_path, ans_name), os.path.join(run_path , ans_name))

def run(): # 运行编译器 获得输出
    subprocess.run([os.path.join(run_path, exe_name)])

def check_same():
    with open(ans_name, "r") as f1:
        with open(mid_name, "r") as f2:
            text1 = f1.readlines()
            text2 = f2.readlines()
    text1 = [i.strip() for i in text1]
    text2 = [i.strip() for i in text2]
    d = difflib.HtmlDiff()
    htmlContent = d.make_file(text1, text2)
    with open(diff_name, "w") as f:
        f.write(htmlContent)
    ratio = difflib.SequenceMatcher(None, text1, text2).ratio()
    if ratio != 1:
        print(f'ratio：{ratio}')
        print(f'出错！测试点路径：{curcase_path}')
        exit(1)

def check_compiler():
    save = os.getcwd()
    os.chdir(run_path)
    run()
    check_same()
    os.chdir(save)

def check_cases_at(dir_path):
    global curcase_path
    os.chdir(dir_path)
    print("更改后的工作目录：", os.getcwd())
    for entry in os.listdir('.'):
        curcase_path = os.path.join(dir_path, entry)
        # os.chdir(curcase_path)
        print(f'当前测试点目录：{curcase_path}')
        build_work_place()
        print(f'构建工作目录成功')
        check_compiler()

# 打开ZIP文件\qlkaicx\article\details\134068734
with zipfile.ZipFile(zip_path, 'r') as zip_ref:
    # 解压文件到指定目录
    zip_ref.extractall(extract_folder)

print("ZIP文件解压完成")

A_path = os.path.join(extract_folder, 'A')
B_path = os.path.join(extract_folder, 'B')
C_path = os.path.join(extract_folder, 'C')

check_cases_at(A_path)
check_cases_at(B_path)
check_cases_at(C_path)
