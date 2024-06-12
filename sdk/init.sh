#echo "src-git skps https://github.com/aszokalski/skps-projekt.git/packages" >> feeds.conf.default
echo "src-link skps $(pwd)/../packages " >> feeds.conf.default
scripts/feeds update -a

