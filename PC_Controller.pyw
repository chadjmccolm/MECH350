# Import the pyserial package to communicate with the arduino
import serial

# Import python native packages necessary to run this program
import sys
import glob
import time

# ser = serial.Serial('COM3', 9600)

# Use Tkinter for python 2, tkinter for python 3
import tkinter as tk
from tkinter import messagebox

# Global Parameters
BACKGROUND_COLOUR = "#4380F9"
FOREGROUND_COLOUR = "#FFFFFF"

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

# Refreshes that list of ports in the list_control object
def refresh_portlist(list_control):

    # Remove all ports from list
    list_control.delete(0, tk.END)

    # Add each port back to the list
    for port in serial_ports():
        list_control.insert(tk.END, port)

# Function for sending the angle to the Arduino
def send_angle(angle, entry):

    # Attempt to convert the user input to a number
    try:
        val = int(angle)

        # If that number is too big or too small throw a ValueError
        if(val > 30 or val < 0):
            raise ValueError

        # Otherwise print that the angle was sent for debugging
        print("sent angle")

        # Change the text entry's value to show the user it has been sent
        entry.delete(0, tk.END)
        entry.insert(0, "Sent! (" + angle + ")")

    # If the ValueError is triggered because it could not be converted to an int or it's out of range
    except ValueError:

        # Show a messagebox with a warning about input values
        messagebox.showwarning(
            "Error",
            "Please input an integer from 0 to 30 degrees."
        )

        # And print that the message was not sent (for debugging)
        print("not sent")
    

# Function for sending the travel to the Arduino
def send_travel(travel, entry):

    # Attempt to convert the user input to a number
    try:
        val = int(travel)

        # If that number is too big or too small throw a ValueError
        if(val > 450 or val < 0):
            raise ValueError

        # Otherwise print that the angle was sent for debugging
        print("sent travel")

        # Change the text entry's value to show the user it has been sent
        entry.delete(0, tk.END)
        entry.insert(0, "Sent! (" + travel + ")")

    # If the ValueError is triggered because it could not be converted to an int or it's out of range
    except ValueError:

        # Show a messagebox with a warning about input values
        messagebox.showwarning(
            "Error",
            "Please input an integer from 0 to 450 mm."
        )

        # And print that the message was not sent (for debugging)
        print("not sent")

# Defines the serial window frame
class SerialWindow(tk.Frame):

    def __init__(self, parent, *args, **kwargs):

        # Initialize the frame
        tk.Frame.__init__(self, parent, background=BACKGROUND_COLOUR, *args, **kwargs)
        self.parent = parent

        # Create a label for the serial list
        serial_label = tk.Label(self, text="Serial List:", background=BACKGROUND_COLOUR, foreground=FOREGROUND_COLOUR, width=8)
        serial_label.grid(row=0, column=0, ipadx=5, ipady=5)

        # Create the serial list
        serial_list = tk.Listbox(self, width=60, height=4)
        serial_list.grid(row=0, column=1, pady=5, padx=5, rowspan=2)

        # Create the button to refresh the list
        serial_refresh = tk.Button(self, text="Refresh", command = lambda: refresh_portlist(serial_list))
        serial_refresh.grid(row=1, column=0)

        # Refresh the list on startup
        refresh_portlist(serial_list)

# Defines the user input space
class EntryWindow(tk.Frame):

    def __init__(self, parent, *args, **kwargs):

        # Initialize the frame
        tk.Frame.__init__(self, parent, background=BACKGROUND_COLOUR, *args, **kwargs)
        self.parent = parent

        # Create the entry for angle
        angle_label = tk.Label(self, text="Angle:", background=BACKGROUND_COLOUR, foreground=FOREGROUND_COLOUR, width=10, font=30)
        angle_label.grid(column=0, row=1)
        angle_entry = tk.Entry(self, width=15, font=30, justify=tk.RIGHT);
        angle_entry.grid(column=1, row=1, pady=5)
        angle_label2 = tk.Label(self, text="deg", background=BACKGROUND_COLOUR, foreground=FOREGROUND_COLOUR, width=9, font=30)
        angle_label2.grid(column=2, row=1)
        angle_button = tk.Button(self, text="Send Angle", command = lambda: send_angle(angle_entry.get(), angle_entry), width=10)
        angle_button.grid(column=3, row=1, padx=20, pady=5)

        # Create the entry for travel
        travel_label = tk.Label(self, text="Travel:", background=BACKGROUND_COLOUR, foreground=FOREGROUND_COLOUR, width=10, font=30)
        travel_label.grid(column=0, row=2)
        travel_entry = tk.Entry(self, width=15, font=30, justify=tk.RIGHT);
        travel_entry.grid(column=1, row=2, pady=5)
        travel_label2 = tk.Label(self, text="mm", background=BACKGROUND_COLOUR, foreground=FOREGROUND_COLOUR, width=3, font=30)
        travel_label2.grid(column=2, row=2)
        travel_button = tk.Button(self, text="Send Travel", command = lambda: send_travel(travel_entry.get(), travel_entry), width=10)
        travel_button.grid(column=3, row=2, padx=20, pady=5)
        
# Defines the main application frame
class MainApplication(tk.Frame):
    
    def __init__(self, parent, *args, **kwargs):

        # Initialize the main frame
        tk.Frame.__init__(self, parent, *args, **kwargs)
        self.parent = parent

        # Create the serial pane
        serial_window = SerialWindow(parent, borderwidth=1, relief=tk.RAISED);
        serial_window.grid(column=0, row=0)

        # Create the entry pane
        entry_window = EntryWindow(parent);
        entry_window.grid(column=0, row=1)
    
# Run the main application
if __name__ == "__main__":
    root = tk.Tk()
    root.title("MECH 350 Controller")
    root.configure(background=BACKGROUND_COLOUR)
    MainApplication(root).grid(row=0, column=0)
    root.mainloop()
