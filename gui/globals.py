from threading import Thread
from ipc import Connection
import os
import psutil
import subprocess

SERVER_IP = "127.0.0.1" # SHOULD ALWAYS BE LOCALHOST!!!
SERVER_PORT = 9745 # Server is hard coded to use this port so it should really never change
KEYS_PER_RANGE = 2**40

key_rate = 0.0
percent_complete = 0.0
conn = None
server_process_name = None

def global_init():
    global conn
    global server_process_name
    
    conn = Connection()

    if "server.exe" in os.listdir():  # server.exe is default server name it should always be this
                                      # otherwise set next found exe in directory to the server process name
        server_process_name = "server.exe"
    else:
        for file_name in os.listdir():
            if file_name[-4:] == ".exe":
                server_process_name = file_name
                
    # terminate process if already running            
    for proc in psutil.process_iter(['pid', 'name']):
        try:
            if proc.info['name'] == server_process_name:
                proc.terminate()
                proc.wait()
        except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
            pass
        
    if server_process_name is not None:
        subprocess.Popen(server_process_name, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        
    return True if server_process_name is not None else False