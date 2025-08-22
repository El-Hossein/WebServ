import subprocess

# Run the curl command and capture output and errors
try:
    result = subprocess.run(
        ["curl", "-i", "http://localhost:8081/cgi/cgiScripts/allo.py"],
        capture_output=True,  # Capture stdout and stderr
        text=True,           # Return output as strings, not bytes
        check=False          # Don't raise an exception on non-zero exit code
    )

    # Check if curl command failed (non-zero exit code)
    if result.returncode != 0:
        print("curl command failed with exit code:", result.returncode)
        print("Error output:", result.stderr)  # Print the error message
    else:
        print("curl command succeeded. Output:")
        print(result.stdout)  # Print the successful output

except subprocess.SubprocessError as e:
    print("Failed to execute curl command:", str(e))