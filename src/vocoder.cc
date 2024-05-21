#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <valarray>
#include <algorithm>
#include <cstdlib>

// Constants
const double PI = 3.14159265358979323846;
const int16_t MAX_INT16 = 32767;

// Type alias for complex numbers
using Complex = std::complex<double>;
using CArray = std::valarray<Complex>;

// Forward FFT
void fft(CArray& x) {
    const size_t N = x.size();
    if (N <= 1) return;

    // Divide
    CArray even = x[std::slice(0, N/2, 2)];
    CArray odd = x[std::slice(1, N/2, 2)];

    // Conquer
    fft(even);
    fft(odd);

    // Combine
    for (size_t k = 0; k < N/2; ++k) {
        Complex t = std::polar(1.0, -2 * PI * k / N) * odd[k];
        x[k] = even[k] + t;
        x[k + N/2] = even[k] - t;
    }
}

// Inverse FFT
void ifft(CArray& x) {
    x = x.apply(std::conj);
    fft(x);
    x = x.apply(std::conj);
    x /= x.size();
}

// Window function
std::vector<double> hanning_window(size_t N) {
    std::vector<double> window(N);
    for (size_t n = 0; n < N; ++n) {
        window[n] = 0.5 * (1 - cos(2 * PI * n / (N - 1)));
    }
    return window;
}

// Short-Time Fourier Transform (STFT)
std::vector<CArray> stft(const std::vector<double>& signal, size_t window_size, size_t hop_size) {
    std::vector<CArray> stft_frames;
    std::vector<double> window = hanning_window(window_size);

    for (size_t i = 0; i + window_size <= signal.size(); i += hop_size) {
        CArray frame(window_size);
        for (size_t j = 0; j < window_size; ++j) {
            frame[j] = signal[i + j] * window[j];
        }
        fft(frame);
        stft_frames.push_back(frame);
    }
    return stft_frames;
}

// Inverse Short-Time Fourier Transform (ISTFT)
std::vector<double> istft(const std::vector<CArray>& stft_frames, size_t window_size, size_t hop_size) {
    std::vector<double> signal(stft_frames.size() * hop_size + window_size, 0.0);
    std::vector<double> window = hanning_window(window_size);

    for (size_t i = 0; i < stft_frames.size(); ++i) {
        CArray frame = stft_frames[i];
        ifft(frame);

        for (size_t j = 0; j < window_size; ++j) {
            signal[i * hop_size + j] += frame[j].real() * window[j];
        }
    }
    return signal;
}

// Phase Vocoder
std::vector<double> phase_vocoder(const std::vector<double>& signal, double time_stretch, int sample_rate) {
    const size_t window_size = 1024;
    const size_t hop_size = window_size / 4;

    std::vector<CArray> stft_frames = stft(signal, window_size, hop_size);
    std::vector<CArray> processed_frames(stft_frames.size());

    // Phase accumulation and processing
    CArray last_phase(window_size);
    CArray new_phase(window_size);

    for (size_t i = 0; i < stft_frames.size(); ++i) {
        for (size_t k = 0; k < window_size; ++k) {
            double magnitude = std::abs(stft_frames[i][k]);
            double phase = std::arg(stft_frames[i][k]);
            double delta_phase = phase - last_phase[k].real();

            // Phase unwrapping
            delta_phase -= 2 * PI * std::round(delta_phase / (2 * PI));
            double true_freq = 2 * PI * k / window_size + delta_phase / hop_size;

            new_phase[k] += true_freq * hop_size * time_stretch;
            processed_frames[i][k] = std::polar(magnitude, new_phase[k].real());
        }
        last_phase = new_phase;
    }

    return istft(processed_frames, window_size, hop_size);
}

// Function to read audio data from standard input
std::vector<double> readAudioData() {
    std::vector<double> data;
    int16_t sample;
    while (std::cin.read(reinterpret_cast<char*>(&sample), sizeof(sample))) {
        data.push_back(static_cast<double>(sample) / MAX_INT16);
    }
    return data;
}

// Function to write audio data to standard output
void writeAudioData(const std::vector<double>& data) {
    for (double sample : data) {
        int16_t output_sample = static_cast<int16_t>(std::clamp(sample, -1.0, 1.0) * MAX_INT16);
        std::cout.write(reinterpret_cast<const char*>(&output_sample), sizeof(output_sample));
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <sample_rate>" << std::endl;
        return 1;
    }
    
    int sample_rate = std::atoi(argv[1]);

    // Read audio data
    std::vector<double> inputData = readAudioData();

    // Time-stretch factor (e.g., 1.5 for 50% slower)
    double time_stretch = 1.5;

    // Process data with phase vocoder
    std::vector<double> outputData = phase_vocoder(inputData, time_stretch, sample_rate);

    // Write processed data
    writeAudioData(outputData);

    return 0;
}
