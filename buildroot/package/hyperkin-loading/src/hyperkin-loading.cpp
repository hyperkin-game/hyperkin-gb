#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <vector>
#include <stdlib.h> //rand()
#include <unistd.h> //usleep
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <stdio.h>
#include <math.h>
#include <getopt.h>
#include <string.h>
#include <asm/param.h>
#include <linux/joystick.h>
#include <dirent.h>
#include <sys/stat.h>

#define PIC_LOADING "/etc/Splash_Screen_MockUp.jpg"
#define DUMPER_PATH "/media/usb0"
#define USERDATA_ROM_PATH "/userdata/rom"
#define USERDATA_PATH "/userdata"
long int g_dumper_total_size = 0;
long int g_avail_size = 0;
long int copied_total_size = 0;
SDL_mutex *copy_mutex = NULL;
int copy_thread_exit = 0;

SDL_Texture* copyright = NULL;
int WIDTH=1280;
int HEIGHT=720;

void ListDrivers(void){
	printf("Testing video drivers...\n");
	std::vector< bool > drivers( SDL_GetNumVideoDrivers() );
	for( int i = 0; i < drivers.size(); ++i ){
		drivers[ i ] = ( 0 == SDL_VideoInit( SDL_GetVideoDriver( i ) ) );
		SDL_VideoQuit();
	}

	printf("SDL_VIDEODRIVER available:");
	for( int i = 0; i < drivers.size(); ++i ){
		printf(" %s", SDL_GetVideoDriver(i));
	}
	printf("\n");

	printf("SDL_VIDEODRIVER usable   :");
	for( int i = 0; i < drivers.size(); ++i ){
		if( !drivers[ i ] ) continue;
			printf(" %s", SDL_GetVideoDriver(i));
	}
	printf("\n");
}

void ListDisplayMode(void){
	int disp_index = 0, i = 0;
	int numDisplayModes = SDL_GetNumDisplayModes(disp_index);
	SDL_DisplayMode displaymode;
	int got = 0;
	
	printf("Number of display modes=%d\n", numDisplayModes);
	for(i = 0; i < numDisplayModes; i++){
		if(SDL_GetDisplayMode(disp_index, i, &displaymode) >= 0){
			printf("%dx%d-%d\n", displaymode.w, displaymode.h, displaymode.refresh_rate);
			if(got == 0){//we use first one resolution.
				WIDTH = displaymode.w;
				HEIGHT = displaymode.h;
				got = 1;
			}
		}
	}
}

static inline int sortbysize(const struct dirent **a, const struct dirent **b){
	int rval;
	struct stat sbuf1, sbuf2;
	char path1[1024], path2[1024];
	//printf("path=[%s], a[%s], b[%s]\n", USERDATA_ROM_PATH, (*a)->d_name, (*b)->d_name);
	sprintf(path1, "%s/%s", USERDATA_ROM_PATH, (*a)->d_name);
	sprintf(path2, "%s/%s", USERDATA_ROM_PATH, (*b)->d_name);

	rval = stat(path1, &sbuf1);
	if(rval != 0){
		printf("stat error[%s]\n", path1); return 0;
	}
	rval = stat(path2, &sbuf2);
	if(rval != 0){
		printf("stat error[%s]\n", path2); return 0;
	}

	return sbuf1.st_size > sbuf2.st_size; //sort from largest size to smallest
}

void CleanUserdata(void){
	DIR* pdir;
	struct dirent **pdirent;
	int n = 0, ret = 0;
	struct stat sbuf;
	char path[1024];
	long int avail_size = g_avail_size;
	pdir = opendir(USERDATA_ROM_PATH);
	if(pdir){
		n = scandir(USERDATA_ROM_PATH, &pdirent, NULL, sortbysize);
		printf("n=%d\n", n);
		while(n--){
			if(!strcmp(pdirent[n]->d_name, ".") || !strcmp(pdirent[n]->d_name, ".."))
				goto free_and_continue;
			if(strstr(pdirent[n]->d_name + strlen(pdirent[n]->d_name) - strlen(".srm"), ".srm"))
				goto free_and_continue;
			if(strstr(pdirent[n]->d_name + strlen(pdirent[n]->d_name) - strlen(".stat"), ".stat"))
				goto free_and_continue;
			sprintf(path, "%s/%s", USERDATA_ROM_PATH, pdirent[n]->d_name);
			stat(path, &sbuf);
			printf("%s[%ld]\n", pdirent[n]->d_name, sbuf.st_size);
			if(avail_size < g_dumper_total_size){
				printf("delete [%s]\n", path);
				ret = remove(path);
				sync();
				avail_size += sbuf.st_size;
			}
free_and_continue:
			free(pdirent[n]);
		}
		free(pdirent);
	}
}

#include <sys/statvfs.h>
long GetAvailableSpace(const char* path){
	struct statvfs stData;

	if((statvfs(path,&stData)) < 0 ) {
		printf("Failed to stat %s:\n", path);
		return -1;
	}else{
		printf("Disk [%s]: available size=%ld bytes\n", path, stData.f_bavail * stData.f_bsize);
		return stData.f_bavail * stData.f_bsize;
	}
}

long int CheckFiles(const char* dumper_folder){
	char *ptr_gb = NULL, *ptr_gba = NULL, *ptr_gbc = NULL, *ptr_sav = NULL, *ptr_crc = NULL;
	struct dirent *dir = NULL;
	int ret1 = 0;
	DIR* di = opendir(dumper_folder);
	long int total_size = 0;
	struct stat st;
	if(di){
		while((dir = readdir(di)) != NULL){
			ptr_gb = strstr(dir->d_name + strlen(dir->d_name) - strlen(".gb"), ".gb");
			ptr_gba = strstr(dir->d_name + strlen(dir->d_name) - strlen(".gba"), ".gba");
			ptr_gbc = strstr(dir->d_name + strlen(dir->d_name) - strlen(".gbc"), ".gbc");
			ptr_sav = strstr(dir->d_name + strlen(dir->d_name) - strlen(".sav"), ".sav");
			ptr_crc = strstr(dir->d_name + strlen(dir->d_name) - strlen(".crc"), ".crc");
			if((ptr_gb || ptr_gba || ptr_gbc || ptr_sav || ptr_crc)){
				char full_name[1024];
				sprintf(full_name, "%s/%s", dumper_folder, dir->d_name);
				stat(full_name, &st);
				total_size += st.st_size;
				printf("found name[%s], size[%ld], total[%ld]\n", dir->d_name, st.st_size, total_size);
                        }
                }
        }
        return total_size;
}

static int CopyFile(const char* src_path, const char* src_name, const char* dst_path, const char* dst_name){
	char src_full_name[1024], dst_full_name[1024];
	int len = 0, src_file = 0, dst_file = 0, ret = 0;
	char buffer[1024];
	sprintf(src_full_name, "%s/%s", src_path, src_name);
	sprintf(dst_full_name, "%s/%s", dst_path, dst_name);

	src_file = open(src_full_name, O_RDONLY | O_NOCTTY);
	if(src_file == -1) { printf("open [%s] fail\n", src_full_name); return -1; }
	else printf("open [%s] success, start read\n", src_full_name);
	dst_file = open(dst_full_name, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if(src_file == -1) { printf("open [%s] fail\n", dst_full_name); close(src_file); return -1; }
	else printf("open [%s] success, start write\n", dst_full_name);
	while (1){//copy file
		char *p, *q;
		len = read(src_file, buffer, sizeof(buffer));
		if(len == 0){//end of file
			break;
		}else{
			if(len < 0){
				printf("[%s]read error: %s\n", src_full_name, strerror(errno));
				ret = -1;
				goto next_file1;
			}
		}
		p = buffer;
		q = buffer + len;
		while ( p < q ){
			len = write(dst_file, p, (size_t)(q - p));
			if( len > 0){
				p += len;
				SDL_LockMutex(copy_mutex);
				copied_total_size += len;
				SDL_UnlockMutex(copy_mutex);
			}else{
				printf("[%s]write error: %s\n", dst_full_name, strerror(errno));
				ret = -1;
				goto next_file1;
			}
		}
	}
next_file1:
	fsync(dst_file);fdatasync(dst_file);
	close(dst_file);
	close(src_file);
	if(len < 0) { printf("remove [%s] file\n", dst_full_name); remove(dst_full_name); }
	printf("Copy to [%s] done\n", dst_full_name);
	return ret;
}

static int CheckSrm(const char* old_path, const char *old_name){
	char old_full_name[1024];
	char new_name[1024];
	char* p = NULL;
	int ret = 0;

	sprintf(old_full_name, "%s/%s", old_path, old_name);
	strcpy(new_name, old_full_name);
	p = strstr(new_name, ".sav");
	if(p != NULL){
		struct stat st;
		strcpy(p, ".srm");
		if(access(new_name, F_OK) != -1){//file exists
			stat(new_name, &st);
			copied_total_size += st.st_size;
			printf("%s file exists, so do not copy\n", new_name);
			return 1;
		}else{//file doesn't exist
			printf("%s file doesn't exists, so copy it\n", new_name);
			return 0;
		}
	}
}

static int SavRenameSrm(const char* old_path, const char *old_name){
	char old_full_name[1024];
	char new_name[1024];
	char* p = NULL;
	sprintf(old_full_name, "%s/%s", old_path, old_name);
	strcpy(new_name, old_full_name);
	p = strstr(new_name, ".sav");
	if(p != NULL){
		strcpy(p, ".srm");
		rename(old_full_name, new_name);
		system("sync");
	}
	return 0;
}

static int CopyThread(void* ptr){
	char *ptr_gb = NULL, *ptr_gba = NULL, *ptr_gbc = NULL, *ptr_sav = NULL, *ptr_crc = NULL;
	struct dirent *dir = NULL;
	int ret1 = 0;
	DIR* di = opendir(DUMPER_PATH);
	struct stat st;
	copied_total_size = 0;
	copy_thread_exit = 0;

	printf("CopyThread Start\n");
	if(di){
		while((dir = readdir(di)) != NULL){//process gb/gba/gbc/sav
			ptr_gb = strstr(dir->d_name + strlen(dir->d_name) - strlen(".gb"), ".gb");
			ptr_gba = strstr(dir->d_name + strlen(dir->d_name) - strlen(".gba"), ".gba");
			ptr_gbc = strstr(dir->d_name + strlen(dir->d_name) - strlen(".gbc"), ".gbc");
			ptr_sav = strstr(dir->d_name + strlen(dir->d_name) - strlen(".sav"), ".sav");
			if(ptr_gb || ptr_gba || ptr_gbc){
				CopyFile(DUMPER_PATH, dir->d_name, USERDATA_ROM_PATH, dir->d_name);
			}
			if(ptr_sav && CheckSrm(USERDATA_ROM_PATH, dir->d_name) == 0){
				if(CopyFile(DUMPER_PATH, dir->d_name, USERDATA_ROM_PATH, dir->d_name) == 0)
					SavRenameSrm(USERDATA_ROM_PATH, dir->d_name);
			}
                }
        }
	di = opendir(DUMPER_PATH);
	if(di){//Only process .crc file
		while((dir = readdir(di)) != NULL){//process crc
			ptr_crc = strstr(dir->d_name + strlen(dir->d_name) - strlen(".crc"), ".crc");
			if(ptr_crc){
				CopyFile(DUMPER_PATH, dir->d_name, USERDATA_ROM_PATH, dir->d_name);
                        }
                }
        }
	printf("CopyThread Exit\n");
	copy_thread_exit = 1;
	return 1;
}

int main( int argc, char** argv ){
	SDL_Thread* copy_thread = NULL;
	int retCopyThread = 0;

	putenv("SDL_MONITOR_INDEX=1");
	printf("SDL_MONITOR_INDEX=1\n");
	g_dumper_total_size = CheckFiles(DUMPER_PATH);
	printf("Check Dumper Files : Total ROM and SAV Size=[%ld]\n", g_dumper_total_size);
	g_avail_size = GetAvailableSpace(USERDATA_PATH);

	if(g_avail_size > 0 && g_avail_size < g_dumper_total_size){
		CleanUserdata();
		GetAvailableSpace(USERDATA_PATH);
	}

	SDL_Init( 0 );

	if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ){
		printf("SDL_Init(): %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}
	printf("SDL_VIDEODRIVER selected : %s\n", SDL_GetCurrentVideoDriver());
	ListDisplayMode();
	SDL_Window* window = SDL_CreateWindow("SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		WIDTH, HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);
	if( NULL == window ){
		printf("SDL_CreateWindow(): %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	printf("SDL_RENDER_DRIVER available:");
	for( int i = 0; i < SDL_GetNumRenderDrivers(); ++i ){
		SDL_RendererInfo info;
		SDL_GetRenderDriverInfo( i, &info );
		printf(" %s", info.name);
	}
	printf("\n");

	SDL_Renderer* renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
	if( NULL == renderer ){
		printf("SDL_CreateRenderer(): %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}
	SDL_RendererInfo info;
	SDL_GetRendererInfo( renderer, &info );
	printf("SDL_RENDER_DRIVER selected : %s\n", info.name);

	int flags = IMG_INIT_JPG | IMG_INIT_PNG;
	int initted = IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
	if((initted&flags) != flags) {
		SDL_Log("IMG_Init: Failed to init required jpg and png support!\n");
		SDL_Log("IMG_Init: %s\n", IMG_GetError());
	}
	copyright = IMG_LoadTexture(renderer, PIC_LOADING);
	float loading_x_ratio = WIDTH / 1920.0;
	float loading_y_ratio = HEIGHT / 1080.0;

	copy_mutex = SDL_CreateMutex();
	copy_thread = SDL_CreateThread(CopyThread, "CopyThread", (void*)NULL);
	if(copy_thread = NULL){
		printf("CopyThread Failed: %s\n", SDL_GetError());
	}

	bool running = true;
	unsigned char i = 0;
	Uint32 start_time = SDL_GetTicks();
	while( running ){
		SDL_Event ev;
		while( SDL_PollEvent( &ev ) ){
			if(( ev.type == SDL_QUIT ) || ( ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE )){
				running = false;
			}
		}
		//Draw hyperkin loading background picture
		SDL_RenderCopyEx(renderer, copyright, NULL, NULL, 0, NULL, SDL_FLIP_NONE);

		//Draw progress bar
		SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
		SDL_Rect rect;
		rect.x = 645 * loading_x_ratio; rect.y = 624 * loading_y_ratio;
		rect.w = (1508 - 645) * loading_x_ratio * ((float)copied_total_size / (float)g_dumper_total_size);
		rect.h = (656 - 624) * loading_y_ratio;
		SDL_RenderFillRect(renderer, &rect);

		SDL_RenderPresent( renderer );

		if(copy_thread_exit == 1) {
			SDL_RenderCopyEx(renderer, copyright, NULL, NULL, 0, NULL, SDL_FLIP_NONE);
			SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
			SDL_Rect rect;
			rect.x = 645 * loading_x_ratio; rect.y = 624 * loading_y_ratio;
			rect.w = (1508 - 645) * loading_x_ratio * ((float)copied_total_size / (float)g_dumper_total_size);
			rect.h = (656 - 624) * loading_y_ratio;
			SDL_RenderFillRect(renderer, &rect);
			SDL_RenderPresent( renderer );
			if((SDL_GetTicks() - start_time) < 3000){
				SDL_Delay(3000 - (SDL_GetTicks() - start_time));
			}
			else
				SDL_Delay(1000);
			running = false;
			break;
		}
		SDL_Delay(10);
	}


	SDL_WaitThread(copy_thread, &retCopyThread);
	SDL_DestroyMutex(copy_mutex);
	SDL_DestroyRenderer( renderer );
	SDL_DestroyWindow( window );
	SDL_Quit();
	return 0;
}
