import hashlib
import json
import os


def list_bin_files():
    """List .bin files in current directory and prompt for .bin and .json file selection"""
    current_dir = os.path.dirname(os.path.abspath(__file__))
    output_dir = os.path.join(current_dir, 'Output')
    
    bin_files = [f for f in os.listdir(output_dir) if f.lower().endswith('.bin')]
    if not bin_files:
        print("No .bin files found in the current directory!")
    else:
        print("Available .bin files:")
        for index, file in enumerate(bin_files, start=1):
            print(f"{index}: {file}")
    
    # Select .bin file
    while True:
        try:
            if bin_files:
                choice = input("Select a .bin file number or enter file path: ")
            else:
                choice = input("Enter .bin file path: ")
            
            # Check if input is a number
            try:
                choice_num = int(choice)
                if not bin_files:
                    print("Invalid choice. Please enter a valid .bin file path!")
                elif 1 <= choice_num <= len(bin_files):
                    bin_file = os.path.join(output_dir, bin_files[choice_num - 1])
                    break
                else:
                    print(f"Invalid choice. Please select 1 to {len(bin_files)} or a valid .bin file path.")
            except ValueError:
                # Check if input is a valid file path
                file_path = choice.strip().strip('"\' ')
                if os.path.isfile(file_path) and file_path.lower().endswith('.bin') and os.path.exists(file_path):
                    bin_file = file_path
                    break
                print("Invalid file path or file is not a .bin file.")
        except KeyboardInterrupt:
            print("\nOperation cancelled by user.")
            return None, None
    
    return bin_file

def calculate_sha256(file_path):
    sha256_hash = hashlib.sha256()
    try:
        with open(file_path, "rb") as file:
            for byte_block in iter(lambda: file.read(4096), b""):
                sha256_hash.update(byte_block)
        return sha256_hash.hexdigest(), None
    except FileNotFoundError:
        return None, "Error: .bin file not found!"
    except Exception as e:
        return None, f"Error: {str(e)}!"

def get_file_size(file_path):
    try:
        return os.path.getsize(file_path), None
    except FileNotFoundError:
        return None, "Error: File not found!"
    except Exception as e:
        return None, f"Error: {str(e)}!"

def read_json_file(json_file):
    try:
        with open(json_file, "r") as file:
            data = json.load(file)
            if not data or not isinstance(data, list) or not data[0]:
                return None, None, None, "Error: Invalid JSON format!"
            record = data[0]
            return (record.get("file_name"), record.get("version"), 
                    record.get("sha256_hash"), record.get("file_size"))
    except FileNotFoundError:
        return None, None, None, "Error: .json file not found!"
    except json.JSONDecodeError:
        return None, None, None, "Error: Invalid JSON file!"
    except Exception as e:
        return None, None, None, f"Error: {str(e)}!"

def is_valid_bin_file(file_path):
    """Check if file has .bin extension"""
    return file_path.lower().endswith('.bin')

def is_valid_json_file(file_path):
    """Check if file has .json extension"""
    return file_path.lower().endswith('.json')

def main():
        
    bin_file = list_bin_files()
        
    output_dir =os.path.dirname(bin_file)

    file_name = os.path.basename(bin_file)
    output_json = os.path.join(output_dir, os.path.splitext(file_name)[0] + '.json')
    
    print(output_json)
        
    file_name, version, stored_hash, stored_size = read_json_file(output_json)
    if stored_size and isinstance(stored_size, str) and stored_size.startswith("Error"):
        print(stored_size)
        return
    
    current_hash, hash_error = calculate_sha256(bin_file)
    current_size, size_error = get_file_size(bin_file)
    
    if current_hash is None:
        print(hash_error)
        return
    if current_size is None:
        print(size_error)
        return
    
    print(f"File name: {file_name}")
    print(f"Version: {version}")
    print(f"Stored size: {stored_size} bytes")
    print(f"Current size: {current_size} bytes")
    print(f"Stored SHA-256: {stored_hash}")
    print(f"Current SHA-256: {current_hash}")
    
    if current_hash == stored_hash and current_size == stored_size:
        print("Result: .bin file matches stored SHA-256 and size.")
    else:
        print("Result: .bin file does NOT match stored SHA-256 or size.")

if __name__ == "__main__":
    main()