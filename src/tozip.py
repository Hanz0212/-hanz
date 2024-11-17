import os
import zipfile

work_path = "D:\c++_code\COMPILER\code\\src"
src_path = work_path
zip_name = 'code.zip'
zip_path = os.path.join(work_path, zip_name)

def zip_cpp_h_files():
    # 检查 zip 文件是否已经存在
    if os.path.exists(zip_path) :
        # 询问用户是否替换现有的 zip 文件
        choice = input(f"{zip_path} 已经存在，是否替换？(y/n): ")
        if choice.lower() == 'y':  
            os.remove(zip_path)  # 删除现有的 ZIP 文件  
            print(f"{zip_path} 已被删除。")  
        else:  
            print("操作已取消。")  
            return  
        
    
    # 创建或替换现有的 zip 文件
    with zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
        # 获取当前目录下的文件列表
        for filename in os.listdir(src_path):
            # 构建完整的文件路径
            file_path = os.path.join(src_path, filename)
            # 检查文件是否是 .cpp 或 .h 文件，且确保是文件而不是目录
            if os.path.isfile(file_path) and (filename.endswith('.cpp') or 
                                              filename.endswith('.h') or 
                                              filename.endswith('config.json') or
                                              filename.endswith('.py') or
                                              filename.startswith('CMakeLists.txt') or
                                              filename.startswith('check') or
                                              filename.startswith('tozip') or
                                              filename.startswith('2024')) and not filename.startswith('test'):
                # 将文件添加到 zip 文件中，保持相对路径
                print(file_path)
                zipf.write(file_path, filename)

    print(f"{zip_path} 已成功创建或替换！")

zip_cpp_h_files()
