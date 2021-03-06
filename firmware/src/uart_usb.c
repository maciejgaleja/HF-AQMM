#include <uart_usb.h>
#include <stm32f1xx_hal_gpio.h>

UART_HandleTypeDef uart_usb_handle;
int uart_input_flag;
char uart_input_buffer_working[UART_USB_INPUT_BUFFER_SIZE];
int uart_input_index;
char uart_input_buffer[UART_USB_INPUT_BUFFER_SIZE];
char uart_input_char;

void uart_usb_gpio_init()
{
	GPIO_InitTypeDef g;

	//	RX pin
	UART_USB_RX_GPIO_RCC_ENABLE();
	g.Mode = GPIO_MODE_AF_INPUT;
	g.Pin = UART_USB_RX_PIN;
	g.Speed = GPIO_SPEED_HIGH;
	g.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(UART_USB_RX_GPIO, &g);

	//	TX pin
	UART_USB_TX_GPIO_RCC_ENABLE();
	g.Mode = GPIO_MODE_AF_PP;
	g.Pin = UART_USB_TX_PIN;
	g.Speed = GPIO_SPEED_HIGH;
	g.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(UART_USB_TX_GPIO, &g);
}

void uart_usb_periph_init()
{
	UART_USB_RCC_ENABLE();
	uart_usb_handle.Instance = USART1;
	uart_usb_handle.State = HAL_UART_STATE_RESET;
	uart_usb_handle.Init.BaudRate = UART_USB_BAUD;
	uart_usb_handle.Init.Mode = USART_MODE_TX_RX;
	uart_usb_handle.Init.Parity = USART_PARITY_NONE;
	uart_usb_handle.Init.StopBits = USART_STOPBITS_1;
	uart_usb_handle.Init.WordLength = USART_WORDLENGTH_8B;
	HAL_UART_Init(&uart_usb_handle);
	uart_usb_handle.State = HAL_UART_STATE_READY;

	HAL_NVIC_SetPriority(UART_USB_IRQn, 0, 1);
	HAL_NVIC_EnableIRQ(UART_USB_IRQn);
}

void uart_usb_init()
{
	uart_usb_gpio_init();
	uart_usb_periph_init();
	uart_input_index = 0;
	uart_usb_start_reception();
}

void uart_usb_send(char *data, uint16_t size)
{
	while (HAL_UART_Transmit(&uart_usb_handle, (uint8_t*) data, size, HAL_MAX_DELAY)
			!= HAL_OK);
}

void uart_usb_send_it(char *data, uint16_t size)
{
	while (HAL_UART_Transmit_IT(&uart_usb_handle, (uint8_t*) data, size)
			!= HAL_OK);
}

void uart_usb_start_reception()
{
	HAL_UART_Receive_IT(&uart_usb_handle, (uint8_t*) (&uart_input_char), 1);
	++uart_input_index;
	if (uart_input_index >= (UART_USB_INPUT_BUFFER_SIZE - 1))
	{
		--uart_input_index;
	}
}

void uart_usb_receive_callback()
{
	uart_input_buffer_working[uart_input_index-1] = uart_input_char;
	uart_input_buffer_working[uart_input_index] = 0;
	if (uart_usb_is_newline_char(
			uart_input_buffer_working[uart_input_index - 1]))
	{
		memcpy(uart_input_buffer, uart_input_buffer_working,
				uart_input_index+1);
		uart_input_index = 0;
		uart_input_flag = 1;
	}
	uart_usb_start_reception();
}

int uart_usb_is_newline_char(char c)
{
	return c == 10 || c == 13;
}

void USART1_IRQHandler()
{
	HAL_UART_IRQHandler(&uart_usb_handle);
}

