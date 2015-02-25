usage="$(basename "$0") [-h] [-V]

where:
    -h  help
    -V  log mais detalhado com resultados, p@10 e map de cada consulta"

logLevel=0
searchType=0

while getopts h:V option; do
  case "$option" in
    h) echo "$usage"
       exit
       ;;
    V) logLevel=1
       ;;
    :) printf "missing argument for -%s\n" "$OPTARG" >&2
       echo "$usage" >&2
       exit 1
       ;;
   \?) printf "illegal option: -%s\n" "$OPTARG" >&2
       echo "$usage" >&2
       exit 1
       ;;
  esac
done
shift $((OPTIND - 1))

. setup.properties

rm a.out

g++ indexer.cpp

./a.out $searchType $logLevel $PATH_PRODUCTS_DESC $FOLDER_CONSULTAS $FOLDER_RELEVANTES