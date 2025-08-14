#!/usr/bin/env python3
"""
Sarior Server - 测试运行器
用于自动化运行和测试所有示例程序
"""

import os
import sys
import subprocess
import time
import signal
from pathlib import Path

class TestRunner:
    def __init__(self):
        self.build_dir = Path(__file__).parent.parent / "build" / "build_output"
        self.bin_dir = self.build_dir / "bin"
        
        # Windows 下的可执行文件路径
        if os.name == 'nt':
            self.bin_dir = self.bin_dir / "RelWithDebInfo"
            self.exe_ext = ".exe"
        else:
            self.exe_ext = ""
    
    def check_build(self):
        """检查是否已构建"""
        if not self.bin_dir.exists():
            print("❌ 构建目录不存在，请先运行构建脚本")
            return False
        
        examples = [
            "01_basic_concepts",
            "02_timers", 
            "03_networking_basics",
            "04_advanced_networking",
            "05_strand_thread_safety"
        ]
        
        missing = []
        for example in examples:
            exe_path = self.bin_dir / f"{example}{self.exe_ext}"
            if not exe_path.exists():
                missing.append(example)
        
        if missing:
            print(f"❌ 缺少可执行文件: {', '.join(missing)}")
            return False
        
        print("✅ 所有示例程序已构建")
        return True
    
    def run_example(self, name, timeout=10):
        """运行单个示例程序"""
        exe_path = self.bin_dir / f"{name}{self.exe_ext}"
        
        print(f"\n🚀 运行 {name}...")
        print("-" * 50)
        
        try:
            # 启动进程
            process = subprocess.Popen(
                [str(exe_path)],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                cwd=str(self.bin_dir)
            )
            
            # 等待指定时间
            try:
                stdout, stderr = process.communicate(timeout=timeout)
                
                if process.returncode == 0:
                    print("✅ 运行成功")
                    if stdout:
                        print("输出:")
                        print(stdout)
                else:
                    print(f"❌ 运行失败 (退出码: {process.returncode})")
                    if stderr:
                        print("错误:")
                        print(stderr)
                        
            except subprocess.TimeoutExpired:
                print(f"⏰ 程序运行超过 {timeout} 秒，正在终止...")
                process.terminate()
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    process.kill()
                print("✅ 程序已终止")
                
        except Exception as e:
            print(f"❌ 运行出错: {e}")
    
    def run_all_examples(self):
        """运行所有示例程序"""
        examples = [
            ("01_basic_concepts", 5),
            ("02_timers", 8),
            ("03_networking_basics", 10),
            ("04_advanced_networking", 15),
            ("05_strand_thread_safety", 10)
        ]
        
        print("🎯 开始运行所有示例程序")
        print("=" * 60)
        
        for name, timeout in examples:
            self.run_example(name, timeout)
            time.sleep(1)  # 短暂暂停
        
        print("\n" + "=" * 60)
        print("🎉 所有示例程序运行完成")
    
    def run_chat_demo(self):
        """运行聊天服务器演示"""
        server_exe = self.bin_dir / f"chat_server{self.exe_ext}"
        client_exe = self.bin_dir / f"chat_client{self.exe_ext}"
        
        if not server_exe.exists() or not client_exe.exists():
            print("❌ 聊天程序未构建")
            return
        
        print("\n💬 启动聊天服务器演示...")
        print("-" * 50)
        print("将启动服务器，请在另一个终端运行客户端")
        print(f"客户端命令: {client_exe}")
        print("按 Ctrl+C 停止服务器")
        
        try:
            subprocess.run([str(server_exe)], cwd=str(self.bin_dir))
        except KeyboardInterrupt:
            print("\n✅ 服务器已停止")

def main():
    runner = TestRunner()
    
    if len(sys.argv) < 2:
        print("Sarior Server 测试运行器")
        print("\n用法:")
        print("  python test_runner.py check     - 检查构建状态")
        print("  python test_runner.py all       - 运行所有示例")
        print("  python test_runner.py chat      - 运行聊天演示")
        print("  python test_runner.py <name>    - 运行指定示例")
        return
    
    command = sys.argv[1]
    
    if command == "check":
        runner.check_build()
    elif command == "all":
        if runner.check_build():
            runner.run_all_examples()
    elif command == "chat":
        runner.run_chat_demo()
    else:
        # 运行指定示例
        if runner.check_build():
            runner.run_example(command)

if __name__ == "__main__":
    main()