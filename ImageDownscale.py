import struct
from PIL import Image
import sys

def convert_to_greyscale(image_path, output_path, size):
    """
    Converts an image to a greyscale image and resizes it to size x size.

    Parameters:
    - image_path: str, path to the input image
    - output_path: str, path to save the output image
    - size: int, the desired width and height of the output image (AxA)
    """
    try:
        # Open the image file
        with Image.open(image_path) as img:
            # Convert the image to greyscale
            grey_img = img.convert('L')
            # Resize the image to AxA
            grey_img_resized = grey_img.resize((size, size))
            # Save the resulting image
            grey_img_resized.save(output_path)
            print(f"Image successfully converted to greyscale and resized to {size}x{size}.")
    except Exception as e:
        print(f"An error occurred: {e}")

def save_greyscale_data_as_text(image_path, output_path):
    """
    Converts an image to 8-bit greyscale and saves the pixel data as a comma-separated stream of numbers in a text file.
    The first line of the file contains the image dimensions (width and height).

    Parameters:
    - image_path: str, path to the input image
    - output_path: str, path to save the output text file
    """
    try:
        # Open the image file
        with Image.open(image_path) as img:
            # Convert the image to greyscale (8-bit)
            grey_img = img.convert('L')
            # Get the image dimensions
            width, height = grey_img.size
            # Get the pixel data
            pixels = list(grey_img.getdata())

            # Write the pixel data to a text file
            with open(output_path, 'w') as file:
                # Write the image dimensions on the first line
                file.write(f"{width},{height}\n")
                # Convert the list of pixel values to a comma-separated string and write to file
                file.write(','.join(map(str, pixels)))
            
            print(f"Greyscale pixel data successfully saved to {output_path}.")
    except Exception as e:
        print(f"An error occurred: {e}")

def save_greyscale_data_as_binary(image_path, output_path):
    """
    Converts an image to 8-bit greyscale and saves the pixel data as a binary file.
    The binary file includes the image dimensions (width and height) followed by the pixel data.

    Parameters:
    - image_path: str, path to the input image
    - output_path: str, path to save the output binary file
    """
    try:
        # Open the image file
        with Image.open(image_path) as img:
            # Convert the image to greyscale (8-bit)
            grey_img = img.convert('L')
            # Get the image dimensions
            width, height = grey_img.size
            # Get the pixel data
            pixels = list(grey_img.getdata())

            # Write the image dimensions and pixel data to a binary file
            with open(output_path, 'wb') as file:
                # Write the width and height as unsigned integers
                file.write(struct.pack('II', width, height))
                # Write the pixel data as unsigned bytes
                file.write(struct.pack(f'{len(pixels)}B', *pixels))
            
            print(f"Greyscale pixel data successfully saved to {output_path}.")
    except Exception as e:
        print(f"An error occurred: {e}")

def read_greyscale_data_from_binary(input_path):
    """
    Reads the image dimensions and pixel data from a binary file.

    Parameters:
    - input_path: str, path to the input binary file

    Returns:
    - width: int, the width of the image
    - height: int, the height of the image
    - pixels: list of int, the greyscale pixel values
    """
    try:
        with open(input_path, 'rb') as file:
            # Read the width and height (each as an unsigned integer)
            width, height = struct.unpack('II', file.read(8))
            # Read the pixel data
            pixel_count = width * height
            pixels = struct.unpack(f'{pixel_count}B', file.read(pixel_count))
            
            print(f"Greyscale pixel data successfully read from {input_path}.")
            return width, height, list(pixels)
    except Exception as e:
        print(f"An error occurred: {e}")
        return None, None, []

# Example usage
# width, height, pixels = read_greyscale_data_from_binary('output_data.bin')
# print(f"Width: {width}, Height: {height}, Number of pixels: {len(pixels)}")


if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python ImageDownscale.py <input_image_path> <output_image_path> <size>")
    else:
        input_image_path = sys.argv[1]
        output_image_path = sys.argv[2]
        size = int(sys.argv[3])
        convert_to_greyscale(input_image_path, output_image_path, size)

        # save_greyscale_data_as_text(input_image_path, output_image_path + '.txt')
        save_greyscale_data_as_binary(input_image_path, output_image_path + '.bin')
