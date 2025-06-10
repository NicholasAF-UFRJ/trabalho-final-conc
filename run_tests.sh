# Arquivo que executa os testes de desempenho dos algoritmos BFS

# Entrada: ./run_tests.sh <número de vértices>

# Saída: Gera um arquivo de resultados.txt com os tempos médios de execução
# Compila os programas necessários e executa os testes de desempenho 
# para diferentes tipos de grafos (todos os grafos menos pequeno e isolado).

# Configurações
VERTICES=$1
TIPOS=(1 2 3 4 6 8 9)  # Ignora 5 (pequeno) e 7 (isolado)
TIPOS_NOMES=("arvore" "arvoreBinaria" "grafoConexo" "arvoreLarga" "grafoCiclo" "grafoCompleto" "grafoLinear")
THREADS=(2 4 8 16)
ARQUIVO_RESULTADOS="resultados.txt"

echo "Resultados de execução ($VERTICES vértices - média de 2 execuções)" > "$ARQUIVO_RESULTADOS"
echo "---------------------------------------------------------------" >> "$ARQUIVO_RESULTADOS"

# Define formato de tempo
TIMEFORMAT='%3R'

make > /dev/null

# Função para calcular média de 2 tempos em segundos com 3 casas decimais
media() {
    printf "%.3f" "$(awk "BEGIN { print ($1 + $2) / 2 }")"
}

# Loop pelos tipos de grafo
for i in "${!TIPOS[@]}"; do
    TIPO=${TIPOS[$i]}
    NOME=${TIPOS_NOMES[$i]}

    echo "Gerando grafo $NOME com $VERTICES vértices..."

    if [[ "$TIPO" -eq 4 ]]; then
        ./geraGrafo $TIPO $VERTICES 10
    else
        ./geraGrafo $TIPO $VERTICES
    fi

    # Executar bfsSeq duas vezes
    echo "Executando bfsSeq em $NOME..."
    TEMPO1=$( { time ./bfsSeq $NOME > /dev/null; } 2>&1 )
    TEMPO2=$( { time ./bfsSeq $NOME > /dev/null; } 2>&1 )
    MEDIA=$(media "$TEMPO1" "$TEMPO2")
    echo "$NOME - bfsSeq: ${MEDIA}s" | tee -a "$ARQUIVO_RESULTADOS"

    # Concorrentes
    for prog in bfsConcEstat bfsConcDin; do
        for t in "${THREADS[@]}"; do
            echo "Executando $prog com $t threads em $NOME..."
            TEMPO1=$( { time ./$prog $NOME $t > /dev/null; } 2>&1 )
            TEMPO2=$( { time ./$prog $NOME $t > /dev/null; } 2>&1 )
            MEDIA=$(media "$TEMPO1" "$TEMPO2")
            echo "$NOME - $prog (${t} threads): ${MEDIA}s" | tee -a "$ARQUIVO_RESULTADOS"
        done
    done

    echo "" >> "$ARQUIVO_RESULTADOS"
done

echo "Todos os testes foram concluídos. Resultados salvos em '$ARQUIVO_RESULTADOS'."