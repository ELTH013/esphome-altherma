#pragma once

#include "esphome/components/uart/uart_component.h"
#include "esphome/core/version.h"
#include <queue>
#include <vector>

namespace esphome
{
    namespace altherma_hub
    {
        static const char *TAG_MOCK = "mock_uart";

        class MockUART : public uart::UARTComponent
        {
        public:
            void write_array(const uint8_t *data, size_t len) override {
                ESP_LOGI(TAG_MOCK, "Write: [%02x %02x %02x %02x]", data[0], data[1], data[2], data[3]);
                generate_response(data[2]);
            }

            size_t available() override {
                return rx_buffer_.size();
            }

            esphome::uart::UARTFlushResult flush() override {
                while (!rx_buffer_.empty()) {
                    rx_buffer_.pop();
                }
                return esphome::uart::UARTFlushResult::UART_FLUSH_RESULT_SUCCESS;
            }
            
            bool read_byte(uint8_t *data) {
                if (rx_buffer_.empty())
                    return false;
                
                *data = rx_buffer_.front();
                rx_buffer_.pop();
                return true;
            }

            bool peek_byte(uint8_t *data) override {
                if (rx_buffer_.empty())
                    return false;
                *data = rx_buffer_.front();
                return true;
            }

            bool read_array(uint8_t *data, size_t len) override {
                if (rx_buffer_.size() < len)
                return false;
                for (size_t i = 0; i < len; i++) {
                data[i] = rx_buffer_.front();
                rx_buffer_.pop();
                }
                return true;
            }

            void check_logger_conflict() override {}

        protected:
            void generate_response(uint8_t reg_id)
            {
                // Simulate processing delay
                delay(10);

                std::vector<uint8_t> response;

                // Generate mock data based on register
                switch (reg_id)
                {
                case 0x60:
                    response = {
                        0x40, 0x60, 0x13, 0x80, 0x00, 0x18, 0x00, 0x00,
                        0x00, 0x00, 0xC2, 0x01, 0xC1, 0x01, 0xE0, 0x02,
                        0x23, 0x91, 0x82, 0x00, 0x17
                    };
                    break;

                case 0x61:
                    // Big-endian 16-bit temperatures (convid 106):
                    // offset 2-3:  R1T LWT before BUH  34.8°C  [0x01, 0x5C]
                    // offset 4-5:  R2T LWT after BUH   36.5°C  [0x01, 0x6D]
                    // offset 6-7:  R3T refrig liquid   25.0°C  [0x00, 0xFA]
                    // offset 8-9:  R4T inlet water     32.0°C  [0x01, 0x40]
                    // offset 10-11: R5T DHW tank       50.0°C  [0x01, 0xF4]
                    // offset 12-13: indoor ambient     21.0°C  [0x00, 0xD2]
                    response = {
                        0x40, 0x61, 0x10, 0x00, 0x00, 0x01, 0x5C, 0x01,
                        0x6D, 0x00, 0xFA, 0x01, 0x40, 0x01, 0xF4, 0x00,
                        0xD2, 0x81
                    };
                    break;
                    

                case 0x21:
                    response = {
                        0x40, 0x21, 0x12, 0xF9, 0x00, 0x95, 0x00, 0xE6, 
                        0x00, 0xA8, 0xCE, 0xFF, 0x67, 0x01, 0x1A, 0x00, 
                        0xC4, 0xFF, 0x00, 0x5E
                    };
                    break;

                default:
                    // Generic response for unknown registers
                    response = {
                        0x40, reg_id, 0x08,
                        0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00};
                    break;
                }

                // Add response to rx buffer
                for (uint8_t byte : response){
                    rx_buffer_.push(byte);
                }

                ESP_LOGI(TAG_MOCK, "Generated response for reg 0x%02x, len=%d", reg_id, response[2]);
            }

            std::queue<uint8_t> rx_buffer_;
        };

    } // namespace altherma_hub
} // namespace esphome