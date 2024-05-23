import os
from datetime import datetime
import time


def write_log(log_message, log_file='log.txt'):
    """
    Write a log message to a log file with a timestamp.
    If the file does not exist, create it. Otherwise, append to the end of the file.

    :param log_message: The message to log
    :param log_file: The log file to write to (default: 'log.txt')
    """
    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    log_entry = f"{timestamp} - {log_message}\n"

    with open(log_file, 'a') as file:
        file.write(log_entry)


def set_system_time(new_time):
    '''
    # Example usage
    new_time = datetime.datetime(2024, 5, 23, 15, 30, 0)
    set_system_time(new_time)
    '''
    # new_time should be a datetime object
    date_str = new_time.strftime('%m-%d-%Y')
    time_str = new_time.strftime('%H:%M:%S')

    # Set the date
    os.system(f'date {date_str}')
    # Set the time
    os.system(f'time {time_str}')


if __name__ == "__main__":
    # Example usage
    write_log("This is a test log message.")
    time.sleep(5)
    write_log("This is another message")
