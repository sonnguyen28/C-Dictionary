#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include "bt-5.0.0/inc/btree.h"

#define MEAN_SIZE 5000
#define WORD_SIZE 40

GtkWidget *textHistory, *textMean, *text_view_edit, *text_view_add;
GtkWidget *searchEntry;
GtkWidget *entry1, *entry2;
GtkWidget *window, *windowAdd, *windowEdit, *windowAbout;
GtkListStore *list;
GtkTreeIter iter;

BTA *book;

char history[MEAN_SIZE], suggest[WORD_SIZE];
int flag, sai = 1;

void openBook();
void showMessage(GtkWidget *parent, GtkMessageType type, char* mms, char* content);
void setMean(char* text);
void setHistory(char *text);

void on_key_pressed(GtkWidget *widget, GdkEventKey *event, gpointer userData);
void checkWord(char* word);
void findMean(char* word);

int main(int argc, char *argv[])
{
    GtkBuilder *builder;
    GtkEntryCompletion *comple;

    gtk_init(&argc, &argv);

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "glade/window_main.glade", NULL);

    window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
    gtk_builder_connect_signals(builder, NULL);

    textHistory = GTK_WIDGET(gtk_builder_get_object(builder, "text_view_history"));
    textMean = GTK_WIDGET(gtk_builder_get_object(builder, "text_view_mean"));
    searchEntry = GTK_WIDGET(gtk_builder_get_object(builder, "search_entry"));

    comple = gtk_entry_completion_new();
    gtk_entry_completion_set_text_column(comple, 0);
    list = gtk_list_store_new(1, G_TYPE_STRING);
    gtk_entry_completion_set_model(comple, GTK_TREE_MODEL(list));
    gtk_entry_set_completion(GTK_ENTRY(searchEntry), comple);

    g_signal_connect(searchEntry, "key_press_event", G_CALLBACK(on_key_pressed), NULL);
    if(!btinit())
        openBook();
    g_object_unref(builder);

    gtk_widget_show(window);
    gtk_main();

    btcls(book);
    return 0;
}

void openBook()
{
    book = btopn("AnhViet.dat", 0, 0);
}

void setMean(char* text)
{
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textMean));
    if(buffer == NULL) {
        printf("Get buffer fail!");
        buffer = gtk_text_buffer_new(NULL);
    }
    gtk_text_buffer_set_text(buffer, text, -1);
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(textMean), buffer);
}

void setHistory(char* text)
{
    GtkTextBuffer *buffer;
    GtkTextIter stIter;
    GtkTextIter edIter;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textHistory));
    gtk_text_buffer_set_text(buffer, text, -1);
    gtk_text_buffer_get_start_iter(gtk_text_view_get_buffer(GTK_TEXT_VIEW(textHistory)), &stIter);
    gtk_text_buffer_get_end_iter(gtk_text_view_get_buffer(GTK_TEXT_VIEW(textHistory)), &edIter);
    char* historyText = gtk_text_buffer_get_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(textHistory)), &stIter, &edIter, FALSE);
    gtk_text_buffer_set_text(buffer, historyText, -1);
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(textHistory), buffer);
}

void on_search_clicked()
{
    char word[WORD_SIZE];
    strcpy(word, gtk_entry_get_text(GTK_ENTRY(searchEntry)));
    findMean(word);
}

void findMean(char* word)
{
    char mean[MEAN_SIZE];
    int size;
    if(btsel(book, word, mean, MEAN_SIZE, &size))
        setMean("\nTừ bạn vừa nhập không có trong từ điển.");
    else {
        setMean(mean);
        strcat(history, word);
        strcat(history, "\n");
        setHistory(history);
    }
}

void on_key_pressed(GtkWidget *widget, GdkEventKey *event, gpointer userData)
{
    char word[WORD_SIZE];
    int x;
    strcpy(word, gtk_entry_get_text(GTK_ENTRY(searchEntry)));
    x = strlen(word);
    if(x == 0) sai = 1;
    if(event->keyval != GDK_KEY_BackSpace) {
        if(event->keyval == GDK_KEY_Tab || event->keyval == GDK_KEY_Return) {
            gtk_entry_set_text(GTK_ENTRY(searchEntry), suggest);
            findMean(suggest);
        }
        else {
            word[x] = event->keyval;
            word[x+1] = '\0';
            flag = 0;
            gtk_list_store_clear(list);
            if(sai!=0) checkWord(word);
        }
    }
    else {
        if(x != 1) {
            word[x - 1] = '\0';
            if(word[0] != '\0') flag = 0;
            sai = 1;
            gtk_list_store_clear(list);
            if(strlen(word) <= strlen(suggest))
                checkWord(word);
        }
    }
}

void checkWord(char* wordCheck)
{
    int lenCheck, size, i, j, first = 0, k = 0, q = 0;
    char listWord[WORD_SIZE], word[WORD_SIZE], mean[MEAN_SIZE];
    char kd[1];
    kd[0] = wordCheck[0];
    kd[1] = '\0';
    lenCheck = strlen(wordCheck);
    flag = 1;
    btsel(book, kd, mean, sizeof(char*), &size);
    while(btseln(book, word, mean, MEAN_SIZE, &size) == 0) {
        i = 0; j = 0;
        if(flag == 0) break;
        while(1) {
            if(flag == 0) break;
            if(wordCheck[0] < word[0]) q = 1;
            if(word[i] == wordCheck[i]) i++;
            else break;
            if(i == lenCheck) {
                j = 1;
                if(first == 0) strcpy(suggest, word);
                first++;
                break;
            }
        }
        if(flag == 0 || q == 1) break;
        sai = 0;
        if(j == 1) {
            sai = 1;
            if(flag == 0) break;
            strcpy(listWord, word);
            gtk_list_store_append(list, &iter);
            gtk_list_store_set(list, &iter, 0, listWord, -1);
            k++;
            if(flag == 0 || k == 15) break;
        }
    }
}

void showMessage(GtkWidget *parent, GtkMessageType type, char* mms, char* content)
{
    GtkWidget *mdialog;
    mdialog = gtk_message_dialog_new(GTK_WINDOW(parent), GTK_DIALOG_DESTROY_WITH_PARENT, type, GTK_BUTTONS_OK, "%s", mms);
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(mdialog), "%s", content);
    gtk_dialog_run(GTK_DIALOG(mdialog));
    gtk_widget_destroy(mdialog);
}

void on_edit_clicked()
{
    GtkBuilder *builder;
    char word[WORD_SIZE];

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "glade/window_main.glade", NULL);

    windowEdit = GTK_WIDGET(gtk_builder_get_object(builder, "window_edit"));
    gtk_builder_connect_signals(builder, NULL);

    entry2 = GTK_WIDGET(gtk_builder_get_object(builder, "word_entry2"));
    text_view_edit = GTK_WIDGET(gtk_builder_get_object(builder, "textEdit"));

    strcpy(word, gtk_entry_get_text(GTK_ENTRY(searchEntry)));
    gtk_entry_set_text(GTK_ENTRY(entry2) ,word);

    g_object_unref(builder);
    gtk_widget_show(windowEdit);
}

void on_btn_edit_clicked()
{
    GtkTextIter stIter, endIter;
    char word[WORD_SIZE];

    strcpy(word, gtk_entry_get_text(GTK_ENTRY(searchEntry)));
    gtk_text_buffer_get_start_iter(gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_edit)), &stIter);
    gtk_text_buffer_get_end_iter(gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_edit)), &endIter);

    char *mean = gtk_text_buffer_get_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_edit)), &stIter, &endIter, FALSE);
    btupd(book, word, mean, strlen(mean) + 1);
    showMessage(windowEdit, GTK_MESSAGE_INFO, "Thành công!", "Sửa từ thành công!");
    gtk_widget_destroy(windowEdit);
}

void on_add_clicked()
{
    GtkBuilder *builder;
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "glade/window_main.glade", NULL);

    windowAdd = GTK_WIDGET(gtk_builder_get_object(builder, "window_add"));
    gtk_builder_connect_signals(builder, NULL);

    entry1 = GTK_WIDGET(gtk_builder_get_object(builder, "word_entry1"));
    text_view_add = GTK_WIDGET(gtk_builder_get_object(builder, "textAdd"));

    g_object_unref(builder);
    gtk_widget_show(windowAdd);
}

void on_btn_add_clicked()
{
    GtkTextIter stIter, endIter;
    char word[WORD_SIZE];
    strcpy(word, gtk_entry_get_text(GTK_ENTRY(entry1)));
    gtk_text_buffer_get_start_iter(gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_add)), &stIter);
    gtk_text_buffer_get_end_iter(gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_add)), &endIter);
    char *mean = gtk_text_buffer_get_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_add)), &stIter, &endIter, FALSE);
    if(btins(book, word, mean, strlen(mean) + 1))
        showMessage(windowAdd, GTK_MESSAGE_ERROR, "Xảy ra lỗi!", "Từ có thể đã tồn tại trong từ điển.");
    else showMessage(windowAdd, GTK_MESSAGE_INFO, "Thành công!", "Đã thêm từ thành công!");
    gtk_widget_destroy(windowAdd);
}

void on_delete_clicked()
{
    char word[WORD_SIZE];
    strcpy(word, gtk_entry_get_text(GTK_ENTRY(searchEntry)));
    btdel(book, word);
    showMessage(window, GTK_MESSAGE_ERROR, "Xóa từ", "Xóa từ thành công!");
}

void on_about_clicked()
{
    GtkBuilder *builder;

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "glade/window_main.glade", NULL);

    windowAbout = GTK_WIDGET(gtk_builder_get_object(builder, "window_infor"));
    gtk_builder_connect_signals(builder, NULL);

    g_object_unref(builder);
    gtk_widget_show(windowAbout);
}

void on_window_main_destroy()
{
    gtk_main_quit();
}
