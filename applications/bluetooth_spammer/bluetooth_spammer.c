```c
#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <stdlib.h>

typedef struct {
    Gui* gui;
    ViewPort* view_port;
    FuriMessageQueue* event_queue;
    NotificationApp* notification;
    bool is_attacking;
    int devices_found;
    int connection_attempts;
} BluetoothSpammerApp;

static void draw_callback(Canvas* canvas, void* ctx) {
    BluetoothSpammerApp* app = ctx;
    char buffer[64];
    
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 10, "BT Spammer");
    
    canvas_set_font(canvas, FontSecondary);
    snprintf(buffer, sizeof(buffer), "Devices: %d", app->devices_found);
    canvas_draw_str(canvas, 0, 25, buffer);
    
    snprintf(buffer, sizeof(buffer), "Attempts: %d", app->connection_attempts);
    canvas_draw_str(canvas, 0, 40, buffer);
    
    if(app->is_attacking) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 0, 55, "SCANNING...");
        canvas_draw_str(canvas, 70, 55, "OK=Stop");
    } else {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 0, 55, "Stopped");
        canvas_draw_str(canvas, 70, 55, "OK=Start");
    }
}

static void input_callback(InputEvent* input_event, void* ctx) {
    BluetoothSpammerApp* app = ctx;
    
    if(input_event->type == InputTypePress) {
        switch(input_event->key) {
        case InputKeyOk:
            app->is_attacking = !app->is_attacking;
            if(app->is_attacking) {
                notification_message(app->notification, &sequence_set_red_255);
                app->devices_found++;
                app->connection_attempts++;
            } else {
                notification_message(app->notification, &sequence_reset_red);
            }
            break;
        case InputKeyBack:
            if(!app->is_attacking) {
                furi_message_queue_put(app->event_queue, input_event, FuriWaitForever);
            }
            break;
        default:
            break;
        }
    }
}

static BluetoothSpammerApp* bluetooth_spammer_app_alloc() {
    BluetoothSpammerApp* app = malloc(sizeof(BluetoothSpammerApp));
    
    app->gui = furi_record_open(RECORD_GUI);
    app->view_port = view_port_alloc();
    app->notification = furi_record_open(RECORD_NOTIFICATION);
    
    view_port_draw_callback_set(app->view_port, draw_callback, app);
    view_port_input_callback_set(app->view_port, input_callback, app);
    
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);
    
    app->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    
    app->is_attacking = false;
    app->devices_found = 0;
    app->connection_attempts = 0;
    
    return app;
}

static void bluetooth_spammer_app_free(BluetoothSpammerApp* app) {
    view_port_enabled_set(app->view_port, false);
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    
    furi_message_queue_free(app->event_queue);
    
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    
    free(app);
}

int32_t bluetooth_spammer_app(void* p) {
    UNUSED(p);
    
    BluetoothSpammerApp* app = bluetooth_spammer_app_alloc();
    
    InputEvent event;
    bool running = true;
    
    while(running) {
        if(furi_message_queue_get(app->event_queue, &event, 100) == FuriStatusOk) {
            if(event.type == InputTypePress && event.key == InputKeyBack) {
                running = false;
            }
        }
        
        if(app->is_attacking) {
            // Имитация поиска устройств
            if(rand() % 100 < 10) { // 10% шанс найти устройство
                app->devices_found++;
                app->connection_attempts++;
            }
        }
        
        view_port_update(app->view_port);
    }
    
    bluetooth_spammer_app_free(app);
    return 0;
}
```
