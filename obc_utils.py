import os
from datetime import datetime, timedelta
import time
import random
import csv


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


def generate_wod(seconds_timestamp: int):
    mode = 1
    bat_voltage = 7 + random.randint(0, 20)*0.05
    bat_current = 0.5 + random.randint(0, 10)*0.00787
    bus_3v_current = 1 + random.randint(0, 10) + 0.025
    bus_5v_current = 1 + random.randint(0, 10) + 0.025
    temp_comm = 33 + random.randint(0, 5)*0.25
    temp_eps = 35 + random.randint(0, 5)*0.25
    temp_battery = 35 + random.randint(0, 5)*0.25

    return [seconds_timestamp, mode, bat_voltage, bat_current, bus_3v_current, bus_5v_current, temp_comm, temp_eps, temp_battery]


def log_wod_data(wod_array, file_path='wod.csv'):
    # Check if the file exists
    file_exists = os.path.isfile(file_path)

    # Open the file in append mode
    with open(file_path, mode='a', newline='') as file:
        writer = csv.writer(file)

        # If the file does not exist, write the headers
        if not file_exists:
            writer.writerow(['time', 'mode', 'bat_voltage', 'bat_current', 'bus_3v_current',
                             'bus_5v_current', 'temp_comm', 'temp_eps', 'temp_battery'])

        # Write the row
        writer.writerow(wod_array)
    write_log("Logged WOD")


def generate_and_log_wod():
    log_wod_data(generate_wod(current_time_to_seconds()))


def print_latest_wod(file_path='wod.csv', num_rows=32):
    if not os.path.exists(file_path):
        print("No WOD logs")
        return
    print('--- LATEST Whole Orbit Data ---')
    rows = []

    # Open the CSV file and read all rows
    with open(file_path, mode='r', newline='') as file:
        reader = csv.reader(file)

        # Extract header
        header = next(reader)

        # Collect all rows in a list
        for row in reader:
            rows.append(row)

    # Determine the starting point to print last `num_rows` rows
    start_index = max(0, len(rows) - num_rows)

    # Print the header
    print(','.join(header))

    # Print the last `num_rows` rows
    for row in rows[start_index:]:
        print(','.join([parse_seconds_to_datetime(int(row[0]))] + row[1:]))
    print('----------------------')


def current_time_to_seconds():
    # Reference epoch (01/01/2000 00:00:00 UTC)
    reference_epoch = datetime(2000, 1, 1, 0, 0, 0)
    # Get the current time
    current_time = datetime.utcnow()
    # Calculate the time difference
    time_difference = current_time - reference_epoch
    # Convert the time difference to seconds
    seconds_since_epoch = int(time_difference.total_seconds())
    return seconds_since_epoch


def parse_seconds_to_datetime(seconds):
    # Reference epoch (01/01/2000 00:00:00 UTC)
    reference_epoch = datetime(2000, 1, 1, 0, 0, 0)
    # Add the given number of seconds to the reference epoch
    target_datetime = reference_epoch + timedelta(seconds=seconds)

    # Format the datetime object
    formatted_datetime = target_datetime.strftime(
        "%I:%M:%S%p %dth %B %Y")  # Example: 6:39PM 26th April 2024
    return formatted_datetime


if __name__ == "__main__":
    # Example usage
    write_log("This is a test log message.")
    time.sleep(5)
    write_log("This is another message")

    generate_and_log_wod()
    time.sleep(5)
    generate_and_log_wod()
    print_latest_wod()
