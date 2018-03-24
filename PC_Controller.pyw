# import the pyserial package to communicate with the arduino
import serial

# import python native packages necessary to run this program
import sys
import glob

# ser = serial.Serial('COM3', 9600)

# Use Tkinter for python 2, tkinter for python 3
import tkinter as tk

# Returns list of all serial ports; borrowed from https://stackoverflow.com/questions/12090503/listing-available-com-ports-with-python
def serial_ports():

    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result

def refresh_portlist(list_control):
    list_control.delete(0, tk.END)
    for port in serial_ports():
        list_control.insert(tk.END, port)

# Defines the serial window frame
class SerialWindow(tk.Frame):

    def __init__(self, parent, *args, **kwargs):

        # Initialize the frame
        tk.Frame.__init__(self, parent, *args, **kwargs)
        self.parent = parent

        # Create a label for the serial list
        serial_label = tk.Label(text="Serial List:")
        serial_label.grid(row=0, column=0, ipadx=5, ipady=5)

        # Create the serial list
        serial_list = tk.Listbox(root, width=100, height=4)
        serial_list.grid(row=0, column=1, pady=5, padx=5, rowspan=2)

        # Create the button to refresh the list
        serial_refresh = tk.Button(root, text="Refresh", command = lambda: refresh_portlist(serial_list))
        serial_refresh.grid(row=1, column=0)

        # Refresh the list on startup
        refresh_portlist(serial_list)

# Defines the main application frame
class MainApplication(tk.Frame):
    
    def __init__(self, parent, *args, **kwargs):

        # Initialize the main frame
        tk.Frame.__init__(self, parent, *args, **kwargs)
        self.parent = parent

        # Create the serial pane
        serial_window = SerialWindow(parent);
        serial_window.grid(column=0, row=0)
        
if __name__ == "__main__":
    root = tk.Tk()
    MainApplication(root).grid(row=0, column=0)
    root.mainloop()
