library(plyr)

le.arquivos <- function(caminho) {
  arquivos <- list.files(caminho, pattern = ".*.csv")
  ldply(arquivos, .fun = function(x) {
    read.csv(file=x, stringsAsFactors=FALSE)
  })
}

join.nl <- function(...) {
  paste(list(...), collapse='\n')
}

rpd <- function(f.method, f.best) {
  abs((f.method - f.best)/f.best) * 100 
}

rpd.reduce <- function(f, col, f.best) {
  f(sapply(col, function(cur) rpd(cur, f.best)))
}

frames <- le.arquivos('.')
result <- ddply(frames, ~ID, summarise, 
                Media.FO=mean(FO), Mediana.FO=median(FO), Max.FO=max(FO), Min.FO=min(FO),
                Media.Tempo=mean(Tempo), Mediana.Tempo=median(Tempo), Min.Tempo=min(Tempo), Max.Tempo=max(Tempo),
                Media.RPD.FO=(function(x) { rpd.reduce(mean, x, max(frames$FO)) })(FO),
                Mediana.RPD.FO=(function(x) { rpd.reduce(median, x, max(frames$FO)) })(FO),
                RPD.Tempo=(function(x) { rpd.reduce(mean, x, min(frames$Tempo)) })(Tempo)
                )

result <- result[order(result$Mediana.RPD.FO, result$Mediana.RPD.Tempo),]
head(result, 5)