import h5py
import numpy

FILENAME = 'seq_t5_n1.h5'

# Edit the shape here:
T = 3
N = 1

indicators = numpy.ones( (T,N) )
for i in range(0,N):
  indicators[0][i] = 0

print indicators

# Open an HDF5 file
h5file = h5py.File( FILENAME, 'w' )

# Set Sequence indicator
dataset = h5file.create_dataset( 'reshape-cm', shape = indicators.shape, dtype = indicators.dtype )
dataset[:] = indicators

h5file.close()
