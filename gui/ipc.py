import socket
import globals

class Connection:
    server = None
    socket = None
    def __init__(self):
        self.server = (globals.SERVER_IP, globals.SERVER_PORT)
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        
    def connect(self) -> bool:
        try:
            self.socket.connect(self.server)
            return True
        except Exception: return False
        
    def init_process(self, process_name: str) -> bool: # returns true if successful
        try:
            self.socket.sendall(process_name.encode("utf-8"))
            return True
        except Exception: return False
        
    def change_update_delay(self, ms: int) -> bool: # returns true if successful
        try:
            buf = f"*{ms}"
            self.socket.sendall(buf.encode("utf-8"))
            return True
        except Exception: return False
    
    def maintain_globals_t(self):
        while True:
            try:
                self.socket.sendall("?".encode("utf-8"))
                data = self.socket.recv(1024).decode("utf-8") # standard, but no real need for 1024 bytes
                print(data)
                key_rate_s = data.split(":")[0]
                percent_complete_s = data.split(":")[1]
                globals.key_rate = float(key_rate_s)
                globals.percent_complete = float(percent_complete_s)
            except Exception:
                while not self.connect():
                    pass
                continue
            