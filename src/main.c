#include <stdio.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <math.h> 



/* пересчет в секунды для типа AVFormatContext */
double format_duration_secs  (AVFormatContext *fmt_ctx)
{
    if (fmt_ctx->duration == AV_NOPTS_VALUE)
        return -1.0;

    // from dump.c -> av_dump_format ()
    // ???
    //int64_t duration = fmt_ctx->duration + (fmt_ctx->duration <= INT64_MAX - 5000 ? 5000 : 0);
    
    return (double) fmt_ctx->duration / AV_TIME_BASE;
}


/* пересчет в секунды для типа AVStream */
double stream_duration_secs (AVStream *stream)
{
    if (stream->duration == AV_NOPTS_VALUE)
        return -1.0;
    return (double) stream->duration * av_q2d(stream->time_base);
}


/* пеерсчет секунды в часы:минуты:секунды.доли */
void print_duration_time (double secs) {

    if (secs <= 0) {
        printf("длительность файла не определена\n");
        return;
    }

    double int_part;
    double frac_part = modf (secs, &int_part);
    
    int64_t isec = (int64_t) int_part;
    int64_t h = isec / 3600;
    isec %= 3600;
    int64_t m = isec / 60;
    isec %= 60;
    printf("%ld ч : %02ld мин : %02.2lf сек\n", h, m, (double) isec + frac_part);
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
    int64_t video_frames = 0;
    double dur_secs = 0.0;

    for (int i = 0; i < stream_count; i++) {

        stream = fmt_ctx->streams[i];
        codecpar = stream->codecpar;
        
        const char *codec_name;
        codec_name = avcodec_get_name(codecpar->codec_id);

        dur_secs = stream_duration_secs (stream);

        switch (codecpar->codec_type) { 

            case AVMEDIA_TYPE_VIDEO: {

                if (stream->nb_frames > 0) {
                    video_frames = stream->nb_frames;
                }
                else if (dur_secs <= 0)
                    video_frames = -1; // 

                else if (  stream->avg_frame_rate.num && stream->avg_frame_rate.den) {
                    double fps = av_q2d(stream->avg_frame_rate);
                    video_frames = llround(fps * dur_secs);
                }
                else if (stream->r_frame_rate.num && stream->r_frame_rate.den) {
                    double fps = av_q2d(stream->r_frame_rate);
                    video_frames = llround(fps * dur_secs);
                }
                else 
                    video_frames = -1;

                printf ("[ПОТОК %d]:\n\tтип = видео, кодек = %s, кол-во кадров = ", i, codec_name);
                if (video_frames < 0)
                    printf ("данные не найдены\n");
                else {
                    total_video_frames += video_frames;
                    printf ("%ld\n", video_frames);
                }
                break;
            } 
            case AVMEDIA_TYPE_AUDIO: {
                printf ("[ПОТОК %d]:\n\tтип = аудио, кодек = %s\n", i, codec_name);
                break;
            } 
            case  AVMEDIA_TYPE_DATA: {
                printf("[ПОТОК %d]:\n\tтип = DATA\n", i);
                break;
            }
            case AVMEDIA_TYPE_SUBTITLE: {
                printf("[ПОТОК %d]:\n\tтип = СУБТИТРЫ\n", i);
                break;
            }
            case AVMEDIA_TYPE_ATTACHMENT: {
                printf("[ПОТОК %d]:\n\tтип = ATTACHMENT\n", i);
                break;
            }
            case AVMEDIA_TYPE_NB: {
                printf("[ПОТОК %d]:\n\tтип = NB\n", i);
                break;
            }
            default: {
                printf("[ПОТОК %d]:\n\tтип = не определен\n", i);
                break;
            }
        }
        printf ("\tдлительность: ");
        print_duration_time (dur_secs);
    }

    printf("\nСуммарное количество кадров во всех видеопотоках: %ld\n", total_video_frames);

    dur_secs = format_duration_secs (fmt_ctx);
    printf ("Длительность файла: ");
    print_duration_time (dur_secs);

    // 
    //printf ("INFO FROM [av_dump_format]:\n\t");
    //for (int i = 0; i < stream_count; i++)
    //    av_dump_format(fmt_ctx, i, filename, 0);

    avformat_close_input(&fmt_ctx);

    return 0;
}