import os
import shutil
import subprocess

settings_file = "/home/adev/Work/2509Detector/RecombinantDetector/RecombinantDetector/settings/user_settings.rec"
logs_dir = "/home/adev/Work/2509Detector/RecombinantDetector/RecombinantDetector/build"

def update_threads_count(threads_count):
    with open(settings_file, 'r') as file:
        lines = file.readlines()

    with open(settings_file, 'w') as file:
        for line in lines:
            if line.startswith("threadsCount"):
                file.write(f"threadsCount {threads_count}\n")
            else:
                file.write(line)


def move_logs(run_number):
    run_dir = f"run_{run_number}"
    if not os.path.exists(run_dir):
        os.makedirs(run_dir)

    for log_file in os.listdir(logs_dir):
        if log_file.endswith(".log"):
            full_log_path = os.path.join(logs_dir, log_file)
            shutil.move(full_log_path, os.path.join(run_dir, log_file))

for i in range(1, 30):
    print(f"Setting threadsCount to {i} and launching program - Run {i}")
    
    update_threads_count(i)

    args = ["./RecDetector", "-detect", "/home/adev/Work/VirusData/dengue_aligned_short_short.fasta"]

    try:
        result = subprocess.run(args, check=True)
        print(f"Run {i} finished with return code {result.returncode}")
        
        move_logs(i)
        print(f"Logs moved to run_{i}")
        
    except subprocess.CalledProcessError as e:
        print(f"Error during execution on Run {i}: {e}")
    except FileNotFoundError:
        print(f"Executable file not found during Run {i}!")
        break
