import os

#command to send to the C++ program 
commands = ["NORMAL", "ERROR", "TAMPER", "OFF", "EXIT"]

print("Avaliable commands: ", commands)

try:
    while True:
        cmd = input("Enter command: ").strip().upper()
        if cmd in commands:
            print(f"Sending command: echo {cmd} | sudo ../src/led_control")
            os.system(f'echo {cmd} | sudo ../src/led_control')
        else:
            print("Invalid command. Try again.")
except KeyboardInterrupt:
    print("Exiting script. ")
    