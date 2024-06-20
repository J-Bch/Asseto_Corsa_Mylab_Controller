from threading import Lock

mutex = Lock()

class Log:
    def __init__( self ):
        self.assetto_corsa = 'NOT CONNECTED'
        self.dashboard = 'NOT CONNECTED'
        self.inputs = False
        print("\n AC TELEMETRY & VIRTUAL GAMEPAD:\n-------------------------------------\n\n\n")

    def print(self):
        mutex.acquire()
        # erase lines        
        print("\33[2K\033[F\033[A")
        print("\33[2K\033[F\033[A")
        
        # print infos
        print("Assetto Corsa link :", self.assetto_corsa, "                      ")
        print("Dashboard link     :", self.dashboard, "                   ")
        if(self.inputs != False):
            print("Last data received:", self.inputs)
                
        mutex.release()

    def update_assetto_status(self, status):
        self.assetto_corsa = status
        self.print()
    
    def update_dashboard_status(self, status):
        self.dashboard = status
        self.print()
    
    def update_inputs_status(self, status):
        self.inputs = status
        self.print()
