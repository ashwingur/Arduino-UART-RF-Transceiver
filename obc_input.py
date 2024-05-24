import threading
import time
from obc_utils import *
import datetime
import random


class InputTimeoutException(Exception):
    pass


def input_with_timeout(prompt, timeout):
    def get_input():
        nonlocal user_input
        user_input = input(prompt)

    user_input = None
    input_thread = threading.Thread(target=get_input)
    input_thread.daemon = True
    input_thread.start()
    input_thread.join(timeout)

    if input_thread.is_alive():
        raise InputTimeoutException(
            "No user input, continuing OBC loops".format(timeout))

    return user_input


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
            else:
                print("invalid command")
        except InputTimeoutException as e:
            print(e)


if __name__ == "__main__":
    main()
