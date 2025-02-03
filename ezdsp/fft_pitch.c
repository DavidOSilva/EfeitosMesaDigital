#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fcomplex.h"
#include "tistdtypes.h"
#define HEADER_SIZE 44    // Tamanho do cabeçalho WAV padrão
#define BUFFER_SIZE 1024  // Tamanho do buffer de leitura compatível com a FFT
#define SAMPLE_RATE 44100 // Taxa de amostragem do áudio
#define FFT_FLAG        0        /* Perform FFT */
#define IFFT_FLAG       1        /* Perform IFFT */
#define SCALE_FLAG      0        /* Scale FFT butterfly output */
#define NOSCALE_FLAG    1        /* Not to scale FFT butterfly output */
#define FILTER_ORDER 3  // Ordem do filtro FIR

#define N 1024           /* Number of FFT points */
#define EXP 10       /* EXP=log2(N) */
#define pi 3.1415926535897  
#define K 3       


complex X[N];           /* Declare input array  */
complex W[EXP];        /* Twiddle e^(-j2pi/N) table */      
complex temp[BUFFER_SIZE]; // Array temporário para espectro alterado

Int16 delayIndex = 0;

// Função para limitar os valores de amostras de áudio
Int16 clamp(Int16 value) {
    if (value > 32767) return 32767;
    if (value < -32768) return -32768;
    return (Int16)value;
}

void bit_rev(complex *X, Int16 exp)
{
    Uint16 i,j,k;
    Uint16 n=1<<exp;	/* Number of points for FFT */
    Uint16 N2=N>>1;
	complex  temp;  /* Temporary storage of complex variable */

    for (j=0,i=1;i<n-1;i++)
    {
        k=N2;
        while(k<=j)
        {
           	j-=k;
            k>>=1;
        }
        j+=k;

        if (i<j)
        {
          temp = X[j];
          X[j] = X[i];
          X[i] = temp;
        }
    }  
}

void fft(complex *X, Uint16 exp, complex *W, Uint16 iFlag, Uint16 sFlag)
{
    complex  U;     /* Twiddle factor W^k */
    complex  temp;  /* Temporary storage of complex variable */

    Uint16 i,j;
    Uint16 id;      /* Index for lower point in butterfly */
    Uint16 L;       /* FFT stage */
    Uint16 LE;      /* Number of points in sub DFT at stage L
                       and offset to next DFT in stage */
    Uint16 LE1;     /* Number of butterflies in one DFT at
                       stage L.  Also is offset to lower point
                       in butterfly at stage L */
    float scale;
    Uint16 n=1<<exp;/* Number of points for FFT */
    
    if (sFlag == 1) /* NOSCALE_FLAG=1 */
        scale = 1.0;
    else            /* SCALE_FLAG = 0 */
        scale = 0.5;/* Scaling of 0.5 at each stage */
     
    if (iFlag == 1) /* FFT_FLAG=0, IFFT_FLAG=1 */
        scale = 1.0;/* Without scaling for IFFT */

    
    for (L=1; L<=exp; L++) 	/* FFT of length 2^EXP */
    {
        LE=1<<L;        	/* LE=2^L=points of sub DFT */
        LE1=LE>>1;      	/* Number of butterflies in sub-DFT */
        U.re = 1.0;
        U.im = 0.0;

        for (j=0; j<LE1;j++)
        {
            for(i=j; i<n; i+=LE) /* Butterfly computations */
            {
                id=i+LE1;
                temp.re = (X[id].re*U.re - X[id].im*U.im)*scale;
                temp.im = (X[id].im*U.re + X[id].re*U.im)*scale;

                X[id].re = X[i].re*scale - temp.re;
                X[id].im = X[i].im*scale - temp.im;

                X[i].re = X[i].re*scale + temp.re;
                X[i].im = X[i].im*scale + temp.im;
            }
            
            /* Recursive compute W^k as U*W^(k-1) */
            temp.re = U.re*W[L-1].re - U.im*W[L-1].im;
            U.im = U.re*W[L-1].im + U.im*W[L-1].re;
            U.re = temp.re;
        }
    }
}

complex interpolate_freq(complex f1, complex f2, float alpha) {
    complex result;
    result.re = f1.re + alpha * (f2.re - f1.re);
    result.im = f1.im + alpha * (f2.im - f1.im);
    return result;
}
int writeIndex = 0;
#define DECAY_FACTOR 0.3
#define DELAY_SIZE 2048
Int16 delayBuffer[DELAY_SIZE];

Int16 apply_reverb(Int16 input) {

    Int16 readIndex;

    readIndex = (writeIndex - 0  + DELAY_SIZE) % DELAY_SIZE;

    Int16 delayedSample = delayBuffer[readIndex];


    Int16 reverbSample = input + (Int16)(delayedSample * DECAY_FACTOR )
                                  + (Int16)(delayBuffer[(readIndex + 500) % DELAY_SIZE] * (DECAY_FACTOR * 0.3))
                                  + (Int16)(delayBuffer[(readIndex + 1000) % DELAY_SIZE] * (DECAY_FACTOR * 0.25))
                                   +  (Int16)(delayBuffer[(readIndex + 1500) % DELAY_SIZE] * (DECAY_FACTOR * 0.12));
    // Atualiza o buffer com o sinal atual
   // delayBuffer[writeIndex] = clampT(reverbSample);
    delayBuffer[writeIndex] = (Int16)(0.7 * delayBuffer[writeIndex]  + (1 - 0.7) * reverbSample);
    //delayBuffer[writeIndex] = reverbSample;
    // Incrementa o índice circularmente
    writeIndex = (writeIndex + 1) % DELAY_SIZE;

    return reverbSample;
}



int main() {
    const char* input_filename = "original.wav";
    const char* output_filename = "testeeeeFFT.wav";
    Uint16 i,j,L,LE,LE1,k,n;


    FILE* inputFile = fopen(input_filename, "rb");
    if (!inputFile) {
        perror("Erro ao abrir o arquivo de entrada");
        return 1;
    }

    FILE* outputFile = fopen(output_filename, "wb");
    if (!outputFile) {
        perror("Erro ao criar o arquivo de saída");
        fclose(inputFile);
        return 1;
    }

    // Ler e copiar o cabeçalho WAV
    unsigned char header[HEADER_SIZE];
    if (fread(header, 1, HEADER_SIZE, inputFile) != HEADER_SIZE ||
        fwrite(header, 1, HEADER_SIZE, outputFile) != HEADER_SIZE) {
        perror("Erro ao processar o cabeçalho do arquivo WAV");
        fclose(inputFile);
        fclose(outputFile);
        return 1;
    }

    // Buffers de entrada e saída
    Int16 input_buffer[BUFFER_SIZE];
    Int16 aux[BUFFER_SIZE];

    float pitch_factor = 1.2f;  // Alterar pitch (1.5x aumenta o pitch)

    for (L=1; L<=EXP; L++)				/* Create twiddle factor table */
    {
        LE=1<<L;						/* LE=2^L=points of sub DFT */
        LE1=LE>>1;						/* Number of butterflies in sub-DFT */
        W[L-1].re = cos(pi/LE1);
        W[L-1].im = -sin(pi/LE1);
    } 

    size_t bytesRead;

    int start_index = 0;
    int end_index = 10000;
    printf("start index: %d", start_index);
    printf("end index: %d", end_index);

    while ((bytesRead = fread(input_buffer, sizeof(Int16), BUFFER_SIZE, inputFile)) > 0) {
        // Converter amostras para números complexos

        for (int i = 0; i < bytesRead; i++) {
            aux[i] = input_buffer[i];
            X[i].re = input_buffer[i];
            X[i].im = 0;
            temp[i].re = 0;
            temp[i].im = 0;
        }

        // Aplicar FFT
        bit_rev(X, EXP);
        fft(X, EXP, W, FFT_FLAG, SCALE_FLAG);

        int new_index = 0;
        float alpha;

        for (int i = 0; i < bytesRead; i++) {  
            // Calcula o novo índice após o pitch shift
           
                //printf("entrou \n");
                float new_index = i * pitch_factor;

                // Converte para índice inteiro para posicionar no array
                int lower_index = (int)new_index;  // Índice inferior
                int upper_index = lower_index + 1; // Próximo índice superior

                // Calcula o peso para interpolação (quanto usar de cada índice)
                float alpha = new_index - lower_index;

                // Verifica se o índice está dentro dos limites do array
                if (lower_index < bytesRead) {
                    // Caso o índice superior também esteja dentro do limite, faz interpolação
                    if (upper_index < bytesRead) {
                        temp[lower_index].re += X[i].re * (1.0 - alpha);  // Parte ponderada do índice inferior
                        temp[lower_index].im += X[i].im * (1.0 - alpha);  // Parte ponderada do índice inferior

                        temp[upper_index].re += X[i].re * alpha;         // Parte ponderada do índice superior
                        temp[upper_index].im += X[i].im * alpha;         // Parte ponderada do índice superior

                    } else {

                        // Se o índice superior está fora do limite, atribui apenas ao índice inferior
                        temp[lower_index].re += X[i].re;
                        temp[lower_index].im += X[i].im;

                    }
                }
            } 
        
      
        for (int i=0; i<bytesRead; i++) /* Change the sign of imaginary part */
        {
            X[i].re = temp[i].re;
            X[i].im = temp[i].im;
            X[i].im = -X[i].im;
        }                      
        
        bit_rev(X,EXP);     /* Arrange sample in bit reversal order */
        fft(X, EXP, W, IFFT_FLAG, SCALE_FLAG);



        // Converter os dados complexos de volta para amostras de áudio
         for (int i = 0; i < N; i++) {
            // X[i].re /= N;
            // X[i].im /= N;
             input_buffer[i] = X[i].re;
             

             input_buffer[i] = apply_reverb(X[i].re);

         }
        // Escrever no arquivo de saída
        
        fwrite(input_buffer, sizeof(Int16), bytesRead, outputFile);

    }
    fclose(inputFile);
    fclose(outputFile);
    printf("Arquivo WAV processado com pitch shift: %s\n", output_filename);
    return 0;
}
