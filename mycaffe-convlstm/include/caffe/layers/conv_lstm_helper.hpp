#ifndef CAFFE_CONV_LSTM_HELPER_HPP
#define CAFFE_CONV_LSTM_HELPER_HPP

#include <boost/lexical_cast.hpp>

namespace caffe{

  template <typename Dtype>
  class ConvLSTMHelper {  
  public:


    static std::string int2str( const int in ) {
      return boost::lexical_cast< std::string >( in );
    }

    static int str2int( const std::string in ) {
      return boost::lexical_cast< int >( in );
    }

    template <typename TP>
    static ConvolutionParameter CopyConv( const TP & l ){
      ConvolutionParameter c;
      c.set_num_output( l.num_output() );  

      for( int i = 0; i < l.kernel_size_size(); i++ )
        c.add_kernel_size( l.kernel_size(i) );
      for( int i = 0; i < l.pad_size(); i++ )
        c.add_pad( l.pad( i ) );
      for( int i = 0; i < l.stride_size(); i++ )
        c.add_stride( l.stride( i ) );
      for( int i = 0; i < l.dilation_size(); i++ )
        c.add_dilation( l.dilation( i ) );

      c.mutable_weight_filler()->CopyFrom( l.weight_filler() );
      c.mutable_bias_filler()->CopyFrom( l.bias_filler() );

      c.set_bias_term( l.bias_term() );
      c.set_axis( l.axis() );

      c.set_group( l.group() );

      c.set_force_nd_im2col( l.force_nd_im2col() );

      c.set_engine( static_cast<ConvolutionParameter_Engine>( l.engine() ) );
      return c;
    }

    static LayerParameter * CreateDummyData( NetParameter* net_param, std::string name, std::string top, BlobShape & shape ){
      LayerParameter * dummy = net_param->add_layer();
      dummy->set_type( "DummyData" );

      if( name != "" )
        dummy->set_name( name );

      dummy->add_top( top );
      BlobShape * new_shape = dummy->mutable_dummy_data_param()->add_shape();
      for( int i = 0; i < shape.dim_size(); i++ )
        new_shape->add_dim( shape.dim(i) );

      return dummy;
    }

    static LayerParameter * CreateInputLayer( NetParameter* net_param, std::string name, std::vector<std::string> names, std::vector<BlobShape> shapes ){
        
      CHECK_EQ( names.size(), shapes.size() );

      LayerParameter * input = net_param->add_layer();
      input->set_type( "Input" );

      if( name != "" )
        input->set_name( name );

      for( int i = 0; i < names.size(); ++i ){
        input->add_top( names[i] );
        input->mutable_input_param()->add_shape()->CopyFrom( shapes[i] );
      }
      return input;
    }

    static LayerParameter * CreateScaleLayer( NetParameter* net_param, std::string name, std::vector<std::string> bottom, std::string top ){
      LayerParameter * scale = net_param->add_layer();
      scale->set_type( "Scale" );
      scale->set_name( name );
      scale->mutable_scale_param()->set_axis(0);
      scale->add_top( top );

      for( auto b : bottom )
          scale->add_bottom( b );

      return scale;   
    }

    static LayerParameter * CreateHadamard( NetParameter * net_param, std::string name, std::string bottom, std::string top, const int axis, const int num_axes  ){
      LayerParameter * hadamard = net_param->add_layer();
      hadamard->set_type( "Scale" );
      hadamard->set_name( name );

      hadamard->mutable_scale_param()->set_axis( axis );
      hadamard->mutable_scale_param()->set_num_axes( num_axes );

      hadamard->add_bottom( bottom );
      hadamard->add_top( top );

      return hadamard;
    }

    static LayerParameter * CreateSliceLayer( NetParameter* net_param, std::string name, std::string bottom, int axis ){
      LayerParameter slice_param;
      slice_param.set_type( "Slice" );

      LayerParameter * slice = net_param->add_layer();
      slice->set_name( name );
      slice->set_type( "Slice" );
      slice->add_bottom( bottom );
      slice->mutable_slice_param()->set_axis( axis );
      return slice;
    }

    static LayerParameter * CreateConcatLayer( NetParameter* net_param, std::string name, std::string top, int axis ){
      LayerParameter * concat = net_param->add_layer();

      concat->set_type( "Concat" );
      concat->set_name( name );
      concat->add_top( top );
      concat->mutable_concat_param()->set_axis( axis );
      return concat;
    }

    static LayerParameter * CreateSumLayer( NetParameter* net_param, std::string name, std::vector<std::string> bottom, std::string top ){
      LayerParameter * sum = net_param->add_layer();
      sum->set_type( "Eltwise" );
      sum->set_name( name );

      sum->mutable_eltwise_param()->set_operation( EltwiseParameter_EltwiseOp_SUM );
      
      for( auto b : bottom )
        sum->add_bottom( b );

      sum->add_top( top );
      
      return sum;
    }

    template <typename TP>
    static LayerParameter * CreateConvLayer( NetParameter* net_param, TP * conv_param, std::string name,
                                                            std::string bottom, std::string top, int axis = 2 ){
      ConvolutionParameter cp = CopyConv( *conv_param );

      LayerParameter * conv = net_param->add_layer();
      conv->set_type("Convolution");

      conv->mutable_convolution_param()->CopyFrom( cp );
      conv->mutable_convolution_param()->set_axis( axis );

      conv->set_name( name );

      conv->add_bottom( bottom );
      conv->add_top( top );

      conv->add_propagate_down( true );

      return conv;
    }

    static LayerParameter * CreateDummyForwardLayer( NetParameter* net_param, std::string name, std::string bottom, std::string top ){
      LayerParameter * dummy = net_param->add_layer();
      dummy->set_type( "DummyForward" );
      dummy->set_name( name );
      dummy->add_bottom( bottom );
      dummy->add_top( top );
      dummy->add_propagate_down( true );
      return dummy;
    }

    static LayerParameter * CreateReductionLayer( NetParameter* net_param, std::string name, std::string bottom, std::string top ){
      LayerParameter* layer = net_param->add_layer();
      layer->set_name( name );
      layer->set_type("Reduction");
      layer->add_bottom( bottom );
      layer->add_top( top );
      return layer;
    }
  };
}

#endif