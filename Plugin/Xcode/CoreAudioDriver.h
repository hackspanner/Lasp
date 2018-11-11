#pragma once

// The LASP driver that handles CoreAudio objects,
// internal filter bank and ring buffers.

#import "Novocaine.h"
#include "Debug.h"
#include "RingBuffer.h"
#include "BiquadFilter.h"

namespace Lasp
{
    class Driver
    {
    public:
        
        Driver()
        {
            audioManager = [Novocaine audioManager];
        }
        
        ~Driver()
        {
        }
        
        bool OpenStream()
        {
            [audioManager setInputBlock:^(float *inputBufferPointer, UInt32 framesPerBuffer, UInt32 numChannels)
            {
                auto inputBuffer = inputBufferPointer;
                auto driver = this;

                auto& buffer_raw = driver->buffers_[0];
                auto& buffer_lpf = driver->buffers_[1];
                auto& buffer_bpf = driver->buffers_[2];
                auto& buffer_hpf = driver->buffers_[3];

                auto& lpf1 = driver->filters_[0];
                auto& lpf2 = driver->filters_[1];
                auto& bpf1 = driver->filters_[2];
                auto& bpf2 = driver->filters_[3];
                auto& hpf1 = driver->filters_[4];
                auto& hpf2 = driver->filters_[5];

                for (auto i = 0u; i < framesPerBuffer; i++)
                {
                    auto input = inputBuffer[i];
                    buffer_raw.pushFrame(input);
                    buffer_lpf.pushFrame(lpf2.feedSample(lpf1.feedSample(input)));
                    buffer_bpf.pushFrame(bpf2.feedSample(bpf1.feedSample(input)));
                    buffer_hpf.pushFrame(hpf2.feedSample(hpf1.feedSample(input)));
                }
            }];
            
            [audioManager play];
            
            // Initialize the filter bank.
            auto fc = 960.0f / audioManager.samplingRate;
            filters_[0].setLowpass(fc, 0.15f);
            filters_[1].setLowpass(fc, 0.15f);
            filters_[2].setBandpass(fc, 0.15f);
            filters_[3].setBandpass(fc, 0.15f);
            filters_[4].setHighpass(fc, 0.15f);
            filters_[5].setHighpass(fc, 0.15f);
            
            return true;
        }
        
        void CloseStream()
        {
            [audioManager pause];
        }
        
        float getSampleRate() const
        {
            return audioManager.samplingRate;
        }
        
        const Lasp::RingBuffer& getBuffer(int index) const
        {
            return buffers_[index];
        }
        
    private:
        
        Novocaine *audioManager;
        
        // Three-band filter bank.
        // We use two filters per band to get 24db/oct slope.
        // The filters are assigned in this order: [LPF1, LPF2, BPF1, BPF2, HPF1, HPF2]
        std::array<BiquadFilter, 6> filters_;
        
        // Ring buffers used for storing filtered results.
        // The buffers are assigned in this order: [non-filtered, low, middle, high]
        std::array<RingBuffer, 4> buffers_;
    };
}
