import subprocess

def run_rpicam_ffmpeg():
    # Define the command
    command = "rpicam-vid -t 0 --inline -o - | ffmpeg -i - -c:v copy -f rtsp rtsp://mediamtx:9554/stream1"

    try:
        # Run the command
        process = subprocess.Popen(command, shell=True)
        process.communicate()  # Wait for the process to finish
    except Exception as e:
        print("Error:", e)

if __name__ == "__main__":
    run_rpicam_ffmpeg()