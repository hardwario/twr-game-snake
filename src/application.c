#include <application.h>

#define MAX_SNAKE_LENGTH 16

#define PIXEL_SIZE 5
#define MAX_X ((128 / PIXEL_SIZE) - 1)
#define MAX_Y ((128 / PIXEL_SIZE) - 1)

#define RIGHT 0
#define TOP 1
#define LEFT 2
#define BOTTOM 3

#define WHITE false
#define BLACK true

Point_t snake[MAX_SNAKE_LENGTH + 1];
int snake_length = 1;
Point_t target;
Point_t next;
int direction = RIGHT;
state_t state;
int menu_item;
int difficulty = 1;

twr_led_t led_lcd_red;
twr_led_t led_lcd_green;
twr_gfx_t *pgfx;

void welcome_page(bool all);
void game_start(void);
void game_over(void);
void game_win(void);
void create_target(void);
void draw_point(Point_t *point, bool color);
void left(void);
void right(void);
void enter(void);
void lcd_button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param);

void application_init(void)
{
    twr_module_lcd_init();

    pgfx = twr_module_lcd_get_gfx();

    static twr_button_t lcd_left;
    twr_button_init_virtual(&lcd_left, TWR_MODULE_LCD_BUTTON_LEFT, twr_module_lcd_get_button_driver(), false);
    twr_button_set_event_handler(&lcd_left, lcd_button_event_handler, NULL);

    static twr_button_t lcd_right;
    twr_button_init_virtual(&lcd_right, TWR_MODULE_LCD_BUTTON_RIGHT, twr_module_lcd_get_button_driver(), false);
    twr_button_set_event_handler(&lcd_right, lcd_button_event_handler, NULL);

    twr_led_init_virtual(&led_lcd_red, TWR_MODULE_LCD_LED_RED, twr_module_lcd_get_led_driver(), true);
    twr_led_init_virtual(&led_lcd_green, TWR_MODULE_LCD_LED_GREEN, twr_module_lcd_get_led_driver(), true);

    welcome_page(true);
}

void application_task(void)
{
	if (state != GAME)
	{
		return;
	}
	if (!twr_module_lcd_is_ready())
	{
		twr_scheduler_plan_current_now();
		return;
	}

	twr_system_pll_enable();

	switch (direction) {
		case RIGHT:
		{
			next.x = snake[0].x + 1;
			next.y = snake[0].y;
			break;
		}
		case LEFT:
		{
			next.x = snake[0].x - 1;
			next.y = snake[0].y;
			break;
		}
		case TOP:
		{
			next.x = snake[0].x;
			next.y = snake[0].y - 1;
			break;
		}
		case BOTTOM:
		{
			next.x = snake[0].x;
			next.y = snake[0].y + 1;
			break;
		}
		default:
			break;
	}

	if (difficulty == 0)
	{
		if (next.x > MAX_X)
		{
			next.x = 0;
		}
		else if (next.x < 0)
		{
			next.x = MAX_X;
		}

		if (next.y > MAX_Y)
		{
			next.y = 0;
		}
		else if (next.y < 0)
		{
			next.y = MAX_Y;
		}
	}
	else
	{
		// check out of map
		if ((next.x > MAX_X) || (next.x < 0) || (next.y > MAX_Y) || (next.y < 0))
		{
			game_over();

            twr_system_pll_disable();

			return;
		}
	}

	// check collision
	for (int i = 0; i < snake_length; i++)
	{
		if ((next.x == snake[i].x) && (next.y == snake[i].y))
		{
			game_over();

            twr_system_pll_disable();

			return;
		}
	}

	draw_point(&snake[snake_length - 1], WHITE);

	for (int i = snake_length -1; i > 0; i--)
	{
		snake[i].x = snake[i - 1].x;
		snake[i].y = snake[i - 1].y;
	}

	snake[0].x = next.x;
	snake[0].y = next.y;

	draw_point(&snake[0], BLACK);

	if ((next.x == target.x) && (next.y == target.y))
	{
		if (snake_length + 1 == MAX_SNAKE_LENGTH)
		{
			game_win();

            twr_system_pll_disable();

			return;
		}

		create_target();

		snake_length++;
	}

	twr_module_lcd_update();

	twr_system_pll_disable();

	twr_scheduler_plan_current_relative((difficulty == HARD ? 50 : 100) - snake_length);
}

void draw_menu_item(int x, int y, char *str, bool color)
{

    int width = twr_gfx_calc_string_width(pgfx, str);

    twr_gfx_draw_fill_rectangle(pgfx, x, y, x + width, y + 15, !color);

    twr_gfx_draw_string(pgfx, x, y, str, color);
}

void welcome_page(bool all)
{
	static char *menu_items[] = {"Easy", "Medium", "Hard"};

	twr_system_pll_enable();

	if (all)
	{
		state = WELCOME;
		menu_item = 0;
		twr_module_lcd_clear();
		twr_scheduler_plan_absolute(0, TWR_TICK_INFINITY);
	}

	twr_module_lcd_set_font(&twr_font_ubuntu_15);

	draw_menu_item(20, 25, "New game", menu_item == 0 ? WHITE : BLACK);

	for (int i = 0; i < 3; i++)
	{
		draw_menu_item(30, 45 + (i * 15), menu_items[i], i == menu_item - 1 ? WHITE : BLACK);

        if (i == difficulty)
        {
            twr_module_lcd_draw_string(20, 45 + (i * 15), "*", BLACK);
        }
        else
        {
            twr_gfx_draw_fill_rectangle(pgfx, 20, 45 + (i * 15), 30, 45 + (i * 15) + 15, WHITE);
        }
	}

	if (all)
	{
		twr_module_lcd_set_font(&twr_font_ubuntu_13);
		twr_module_lcd_draw_string(5, 115, "Down", BLACK);
		twr_module_lcd_draw_string(90, 115, "Enter", BLACK);
	}

	twr_module_lcd_update();

	twr_system_pll_disable();

	twr_scheduler_plan_absolute(0, TWR_TICK_INFINITY);
}

void game_start()
{
	twr_system_pll_enable();

	memset(snake, 0x00, sizeof(snake));

	twr_module_lcd_clear();

	snake_length = 1;
	direction = 0;

	snake[0].x = (uint16_t)rand() % MAX_X;
	snake[0].y = (uint16_t)rand() % MAX_Y;

	draw_point(&snake[0], BLACK);

	create_target();

	twr_module_lcd_update();

	twr_scheduler_plan_now(0);

	state = GAME;

	twr_system_pll_disable();
}

void game_over(void)
{
	twr_system_pll_enable();

    twr_led_pulse(&led_lcd_red, 100);

	twr_module_lcd_set_font(&twr_font_ubuntu_24);
	twr_module_lcd_draw_string(20, 40, "GAME", true);
	twr_module_lcd_draw_string(40, 65, "OVER", true);

	twr_module_lcd_set_font(&twr_font_ubuntu_13);
	twr_module_lcd_draw_string(20, 110, "Press any key ...", true);

	twr_module_lcd_update();

	state = END;

	twr_system_pll_disable();
}

void game_win(void)
{
	twr_system_pll_enable();

    twr_led_pulse(&led_lcd_green, 100);

	twr_module_lcd_set_font(&twr_font_ubuntu_24);
	twr_module_lcd_draw_string(10, 40, "YOU WON", true);

	twr_module_lcd_set_font(&twr_font_ubuntu_13);
	twr_module_lcd_draw_string(20, 110, "Press any key ...", true);

	twr_module_lcd_update();

	state = END;

	twr_system_pll_disable();
}

void create_target(void)
{
	bool bad;

	do {
		target.x = rand() % MAX_X;
		target.y = rand() % MAX_Y;
		bad = false;

		for (int i = 0; i < snake_length; i++)
		{
			if ((target.x == snake[i].x) && (target.y == snake[i].y))
			{
				bad = true;
				break;
			}
		}

	} while (bad);

	draw_point(&target, BLACK);
}

void draw_point(Point_t *point, bool color)
{
	int x = point->x * PIXEL_SIZE;
	int y = point->y * PIXEL_SIZE;

	for (int i = 0; i < PIXEL_SIZE; i++)
	{
		for (int j = 0; j < PIXEL_SIZE; j++)
		{
			twr_module_lcd_draw_pixel(x + i, y + j, color);
		}
	}
}

void left(void)
{
	switch (state) {
		case GAME:
		{
			if (++direction > 3)
			{
				direction = 0;
			}
			return;
		}
		case WELCOME:
		{
			if (++menu_item == 4)
			{
				menu_item = 0;
			}
			welcome_page(false);
			return;
		}
        case END:
		default:
		{
			welcome_page(true);
			return;
		}
	}
}

void right(void)
{
	switch (state) {
		case GAME:
		{
			if (--direction < 0)
			{
				direction = 3;
			}
			return;
		}
		case WELCOME:
		{
			enter();
			return;
		}
        case END:
		default:
		{
			welcome_page(true);
			return;
		}
	}
}

void enter(void)
{
	if (state == WELCOME)
	{
		if (menu_item == 0)
		{
			game_start();
		}
		else if (menu_item > 0)
		{
			difficulty = menu_item - 1;
			welcome_page(false);
		}
	}
}

void lcd_button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    (void) event_param;

	if (event == TWR_BUTTON_EVENT_PRESS)
	{
		if (self->_channel.virtual == TWR_MODULE_LCD_BUTTON_LEFT)
		{
			left();
		}
		else
		{
			right();
		}
	}
	else if (event == TWR_BUTTON_EVENT_HOLD)
	{
		welcome_page(true);
	}
}
