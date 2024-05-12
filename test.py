
from datetime import datetime


def parse_datetime_to_seconds(datetime_str):
    # Parse the datetime string into a datetime object
    dt = datetime.strptime(datetime_str, "%d-%m-%Y-%H-%M-%S")
    # Reference epoch (01/01/2000 00:00:00 UTC)
    reference_epoch = datetime(2000, 1, 1, 0, 0, 0)
    # Calculate the time difference
    time_difference = dt - reference_epoch
    # Convert the time difference to seconds
    seconds_since_epoch = int(time_difference.total_seconds())
    return seconds_since_epoch

datetime_str = "18-04-2024-16-20-17"  # Example datetime string
seconds_since_epoch = parse_datetime_to_seconds(datetime_str)
print(seconds_since_epoch)