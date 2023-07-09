#include <iostream>
#include <SDL2/SDL.h>
#include <cmath>

#define WIDTH 640
#define HEIGHT 480
#define ORTOGONAL 0
#define PERSPECTIVA 1

typedef struct _tObj {
    int nPontos;
    int nArestas;
    float **pontos;
    int **arestas;
    float **modelMatrix;
} tObjeto3d;

typedef struct _tCam {
    float pos[3];
    float centro[3];
    float cima[3];
    float **viewMatrix;
} tCamera;

typedef struct _tProj {
    int tipo;
    float left;
    float right;
    float top;
    float bottom;
    float near;
    float far;
    float **projectionMatrix;
} tProj;

void criaIdentidade4d(float **novaMatriz) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (i == j)
                novaMatriz[i][j] = 1.0;
            else
                novaMatriz[i][j] = 0.0;
        }
    }
}

void imprimeMatriz(float **matriz) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            std::cout << matriz[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

float *multMatrizPonto4d(float **matriz, float *ponto) {
    float *resultado = new float[4];

    for (int i = 0; i < 4; i++) {
        resultado[i] = 0.0;
        for (int j = 0; j < 4; j++) {
            resultado[i] += matriz[i][j] * ponto[j];
        }
    }

    return resultado;
}

void MultMatriz4d(float **matrizA, float **matrizB) {
    float matrizC[4][4];
    float soma;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            soma = 0.0;
            for (int k = 0; k < 4; k++) {
                soma += matrizA[i][k] * matrizB[k][j];
            }
            matrizC[i][j] = soma;
        }
    }

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            matrizB[i][j] = matrizC[i][j];
        }
    }
}

tProj *criaProjecao(int tipo, float left, float right, float top, float bottom, float near, float far) {
    tProj *novaProjecao = new tProj;

    novaProjecao->projectionMatrix = new float *[4];
    for (int i = 0; i < 4; i++) {
        novaProjecao->projectionMatrix[i] = new float[4];
        for (int j = 0; j < 4; j++) {
            novaProjecao->projectionMatrix[i][j] = 0.0;
        }
    }
    novaProjecao->projectionMatrix[0][0] = 2.0 / 16.0;
    novaProjecao->projectionMatrix[1][1] = 2.0 / 12.0;
    novaProjecao->projectionMatrix[2][2] = -2.0 / -19.0;
    novaProjecao->projectionMatrix[3][3] = 1.0;
    novaProjecao->projectionMatrix[2][3] = 1.0;

    return novaProjecao;
}

tCamera *criaCamera() {
    tCamera *novaCamera = new tCamera;

    novaCamera->viewMatrix = new float *[4];
    for (int i = 0; i < 4; i++) {
        novaCamera->viewMatrix[i] = new float[4];
    }

    novaCamera->pos[0] = 0.0;
    novaCamera->pos[1] = 0.0;
    novaCamera->pos[2] = 1.0;

    novaCamera->centro[0] = 0.0;
    novaCamera->centro[1] = 0.0;
    novaCamera->centro[2] = 0.0;

    novaCamera->cima[0] = 0.0;
    novaCamera->cima[1] = 1.0;
    novaCamera->cima[2] = 0.0;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (i == j)
                novaCamera->viewMatrix[i][j] = 1.0;
            else if (i == 2 && j == 3)
                novaCamera->viewMatrix[i][j] = -1.0;
            else
                novaCamera->viewMatrix[i][j] = 0.0;
        }
    }

    return novaCamera;
}

void desenhaArestaViewport(SDL_Renderer *renderer, float *ponto1, float *ponto2) {
    int p1x = ((ponto1[0]) + 1) * WIDTH / 2;
    int p1y = (1 - ponto1[1]) * HEIGHT / 2;
    int p2x = ((ponto2[0]) + 1) * WIDTH / 2;
    int p2y = (1 - ponto2[1]) * HEIGHT / 2;

    SDL_RenderDrawLine(renderer, p1x, p1y, p2x, p2y);
}

tObjeto3d *carregaObjeto(const char *nomeArquivo) {
    FILE *arquivoObj = fopen(nomeArquivo, "r");
    if (arquivoObj == NULL) {
        printf("Erro ao abrir arquivo\n");
        return NULL;
    }

    tObjeto3d *novoObjeto = new tObjeto3d;

    fscanf(arquivoObj, "%d", &(novoObjeto->nPontos));

    novoObjeto->pontos = new float *[novoObjeto->nPontos];
    for (int i = 0; i < novoObjeto->nPontos; i++) {
        novoObjeto->pontos[i] = new float[3];
        fscanf(arquivoObj, "%f%f%f", &(novoObjeto->pontos[i][0]), &(novoObjeto->pontos[i][1]), &(novoObjeto->pontos[i][2]));
    }

    fscanf(arquivoObj, "%d", &(novoObjeto->nArestas));
    novoObjeto->arestas = new int *[novoObjeto->nArestas];
    for (int i = 0; i < novoObjeto->nArestas; i++) {
        novoObjeto->arestas[i] = new int[2];
        fscanf(arquivoObj, "%d%d", &(novoObjeto->arestas[i][0]), &(novoObjeto->arestas[i][1]));
    }

    novoObjeto->modelMatrix = new float *[4];
    for (int i = 0; i < 4; i++) {
        novoObjeto->modelMatrix[i] = new float[4];
        for (int j = 0; j < 4; j++) {
            if (i == j)
                novoObjeto->modelMatrix[i][j] = 1.0;
            else
                novoObjeto->modelMatrix[i][j] = 0.0;
        }
    }

    return novoObjeto;
}

void imprimeObjeto(tObjeto3d *objeto) {
    if (objeto == NULL) {
        printf("ERRO: Objeto nulo!\n");
        return;
    }

    printf("Pontos\n");
    for (int i = 0; i < objeto->nPontos; i++) {
        printf(" [%d] - (%8.4f, %8.4f, %8.4f)\n", i, objeto->pontos[i][0], objeto->pontos[i][1], objeto->pontos[i][2]);
    }

    printf("Arestas\n");
    for (int i = 0; i < objeto->nArestas; i++) {
        printf(" [%d] - (%3d, %3d)\n", i, objeto->arestas[i][0], objeto->arestas[i][1]);
    }

    printf("modelMatrix\n");
    imprimeMatriz(objeto->modelMatrix);
}

void desenhaObjeto(SDL_Renderer *renderer, float **matriz, tObjeto3d *objeto) {
    for (int i = 0; i < objeto->nArestas; i++) {
        float *ponto1 = multMatrizPonto4d(matriz, objeto->pontos[objeto->arestas[i][0]]);
        float *ponto2 = multMatrizPonto4d(matriz, objeto->pontos[objeto->arestas[i][1]]);
        desenhaArestaViewport(renderer, ponto1, ponto2);
        delete[] ponto1;
        delete[] ponto2;
    }
}

int main(int argc, char *argv[]) {
    SDL_Window *window;
    SDL_Event windowEvent;
    SDL_Renderer *renderer;
    tObjeto3d *objeto1;
    tCamera *camera1;
    tProj *projecao1;
    float **matrizComposta;
    int quit = 0;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erro ao inicializar SDL! SDL Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    window = SDL_CreateWindow("Hello SDL World", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);
    if (window == NULL) {
        printf("Erro ao criar janela! SDL Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);

    objeto1 = carregaObjeto("cubo.dcg");
    imprimeObjeto(objeto1);

    camera1 = criaCamera();
    projecao1 = criaProjecao(0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

    matrizComposta = new float *[4];
    for (int i = 0; i < 4; i++) {
        matrizComposta[i] = new float[4];
    }

    while (!quit) {
        SDL_Delay(10);
        SDL_PollEvent(&windowEvent);

        switch (windowEvent.type) {
            case SDL_QUIT:
                quit = 1;
                break;
            // TODO input handling code goes here
        }

        // clear window
        SDL_SetRenderDrawColor(renderer, 242, 242, 242, 255);
        SDL_RenderClear(renderer);

        // TODO rendering code goes here
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        printf("Cria identidade...\n");
        criaIdentidade4d(matrizComposta);
        imprimeMatriz(matrizComposta);

        printf("Multiplicando matrizes Model X Id...\n");
        MultMatriz4d(objeto1->modelMatrix, matrizComposta);
        imprimeMatriz(matrizComposta);

        printf("Multiplicando matrizes View X Model...\n");
        MultMatriz4d(camera1->viewMatrix, matrizComposta);
        imprimeMatriz(matrizComposta);

        printf("Multiplicando matrizes Projecao X View X Model...\n");
        MultMatriz4d(projecao1->projectionMatrix, matrizComposta);
        imprimeMatriz(matrizComposta);

        printf("Desenhando objeto...\n");
        desenhaObjeto(renderer, matrizComposta, objeto1);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}