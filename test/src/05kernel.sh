
cvmfs_test_name="Linux Kernel Compile"

cvmfs_run_test() {
  logfile=$1

  setup_atlaslhcb || return 1

  logdir=`dirname $logfile`
  full_logpath="`cd $logdir; pwd`/`basename $logfile`"
  rm -rf /tmp/kbuild
  cd /cvmfs/atlas.cern.ch/test/linux
  ./compileKernel.sh 2.6.18.8 /tmp/kbuild 8 >> $full_logpath 2>&1 || return 2
  ./compileKernel.sh 2.6.18.8 /tmp/kbuild 8 >> $full_logpath 2>&1 || return 3
  cvmfs-talk -i atlas cleanup 0 >> $full_logpath || return 4 
  ./compileKernel.sh 2.6.18.8 /tmp/kbuild 8 >> $full_logpath 2>&1 || return 5

  check_memory atlas 30000 || return 6 

  return 0
}
