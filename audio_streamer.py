import wave
import subprocess
import struct

def stream_audio_to_vocoder(wav_file, vocoder_executable):
    with wave.open(wav_file, 'rb') as wf:
        sampwidth = wf.getsampwidth()
        num_channels = wf.getnchannels()
        framerate = wf.getframerate()

        # Launch the vocoder executable
        process = subprocess.Popen([vocoder_executable, str(framerate)], stdin=subprocess.PIPE, stdout=subprocess.PIPE)

        # Read and stream audio data to the vocoder
        while True:
            frames = wf.readframes(1024)
            if len(frames) == 0:
                break

            if num_channels > 1:
                frames = struct.unpack('<' + 'h' * (len(frames) // (sampwidth // 2)), frames)
                frames = frames[::num_channels]  # Extract left channel
                frames = struct.pack('<' + 'h' * len(frames), *frames)

            process.stdin.write(frames)
        
        # Close stdin to indicate end of input
        process.stdin.close()

        # Read and process the output from the vocoder
        output_data = b""
        while True:
            output_chunk = process.stdout.read(4096)
            if len(output_chunk) == 0:
                break
            output_data += output_chunk

        process.stdout.close()

        # Write the output to a new .wav file for verification
        with wave.open('output.wav', 'wb') as out_wf:
            out_wf.setnchannels(1)
            out_wf.setsampwidth(sampwidth)
            out_wf.setframerate(framerate)
            out_wf.writeframes(output_data)

if __name__ == "__main__":
    stream_audio_to_vocoder('test_data/Recording.wav', './vocoder')
