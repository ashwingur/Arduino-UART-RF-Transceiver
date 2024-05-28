from obc_utils import *
import datetime
import random
import os

def main():
    while True:
        try:
            user_input: str = input("Enter command: ")
            print(f"User input: {user_input}")
            write_log(f'User entered command: {user_input}')
            if user_input == "ping":
                write_log("Ping received, replying with pong")
                print("pong")
            elif user_input == "wod":
                write_log("Downlinking latest WOD")
                print_latest_wod()
            elif user_input == "logwod":
                write_log("Logging WOD")
                generate_and_log_wod()
            elif user_input == "gettime":
                write_log("Get time request received")
                print(
                    f"current time: {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

            elif "settime" in user_input:
                args = user_input.split()
                if len(args) != 2:
                    print(
                        "Incorrect settime args provided. Usage: settime yyyy-mm-dd-HH-MM-SS")
                    continue
                time_values = args[1].split('-')
                time_values = list(map(int, time_values))
                print(time_values)
                set_system_time(datetime.datetime(*time_values))
                write_log(f"Setting time to {args[1]}")
            elif user_input == "getcurrent":
                payload_current = 0.235 + random.randint(0, 100)*0.00015353
                print(
                    f"Payload current value: {payload_current}A")
                write_log(f"Requested payload current ({payload_current}A)")
            elif user_input == "payloadstrike":
                print("Payload strike activated")
                write_log(f"Payload strike activated")
            elif user_input == "payloadimage":
                print("Taking a payload image")
                write_log(f"Payload image taken")
            elif user_input == "clearstorage":
                print("Clearing storage")
                if os.path.exists('wod.csv'):
                    os.remove('wod.csv')
                write_log(f"Clearing WOD logs")

            else:
                print("invalid command")
        except Exception as e:
            print(e)
            write_log(e)


if __name__ == "__main__":
    main()
