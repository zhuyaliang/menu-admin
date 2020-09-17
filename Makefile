CC = gcc  
MAINC =main.c app-menu.c app-util.c menu-window.c
EXEC = style
CFLAGS = `pkg-config --cflags --libs gtk+-3.0 libmate-menu` -DMATEMENU_I_KNOW_THIS_IS_UNSTABLE
main: 
	$(CC)  -g $(MAINC)  -o $(EXEC) $(CFLAGS)
clean:
	rm $(EXEC) -rf
