#include <stdio.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>




/* пересчет в секунды для типа AVFormatContext */
double format_duration_secs  (AVFormatContext *fmt_ctx)
{
    return (double) fmt_ctx->duration / AV_TIME_BASE;
}


/* пересчет в секунды для типа AVStream */
double stream_duration_secs (AVStream *stream)
{
    return (double) stream->duration * av_q2d(stream->time_base);
}


/* пеерсчет секунды в часы:минуты:секунды */
void print_duration_time (double secs) {

    if (secs <= 0) {
        printf("Длительность файла не определена\n");
        return;
    }

    // ???
    // не зная для каких целей нужно время, не понятно в какую сторону округлять
    //secs += 0.9999999999999999; // округление вверх
    
    int64_t isec = (int64_t) secs;
    int64_t h = isec / 3600;
    isec %= 3600;
    int64_t m = isec / 60;
    isec %= 60;
    printf("%ld ч : %02ld мин : %02ld сек\n", h, m, isec);
}


int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("не указан файл\n");
        return 1;
    }

    const char *filename = argv[1];
    AVFormatContext *fmt_ctx = NULL;
    AVStream *stream = NULL;
    AVCodecParameters *codecpar = NULL;

    if (avformat_open_input(&fmt_ctx, filename, NULL, NULL) != 0) {
        printf("Ошибка: Не удалось открыть файл %s\n", filename);
        return 1;
    }

    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        printf("Ошибка: Не удалось найти информацию о потоках в файле\n");
        avformat_close_input(&fmt_ctx);
        return 1;
    }

    int stream_count = fmt_ctx->nb_streams;
    printf("Количество обнаруженных потоков: %d\n", stream_count);

    int64_t total_video_frames = 0;
    double dur_secs = 0.0;

    for (int i = 0; i < stream_count; i++) {

        stream = fmt_ctx->streams[i];
        codecpar = stream->codecpar;
        
        const char *codec_name;
        codec_name = avcodec_get_name(codecpar->codec_id);

        if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {

            total_video_frames += stream->nb_frames;
            printf ("[ПОТОК %d]: видео, кодек = %s, кол-во кадров = %ld\n", i, codec_name, stream->nb_frames);
        } else if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            printf ("[ПОТОК %d]: аудио, кодек = %s\n", i, codec_name);
        } else {
            printf("[ПОТОК %d] - неизвестный тип потока\n", i);
        }
        dur_secs = stream_duration_secs (stream);
        printf ("Длительность [ПОТОК %d]: ", i);
        print_duration_time (dur_secs);
    }

    printf("Суммарное количество кадров во всех видеопотоках: %ld\n", total_video_frames);

    dur_secs = format_duration_secs (fmt_ctx);
    printf ("Длительность файла: ");
    print_duration_time (dur_secs);

    avformat_close_input(&fmt_ctx);

    return 0;
}
