#include "caffe/layers/conv_lstm_layer.hpp"

namespace caffe {

  template <typename Dtype>
  void ConvLSTMLayer<Dtype>::Reshape(const vector<Blob<Dtype>*>& bottom,
        const vector<Blob<Dtype>*>& top) {
    RecurrentLayer<Dtype>::Reshape( bottom, top );

    // Recurrent Layer does not share diff. for exposed hidden and cell state. Do so here.
    if( this->expose_hidden_ ){
      const int bottom_offset = 2 + this->static_input_;
      for (int i = bottom_offset, j = 0; i < bottom.size(); ++i, ++j) {
        this->recur_input_blobs_[j]->ShareDiff(*bottom[i]);
      }

      const int top_offset = this->output_blobs_.size();
      for (int i = top_offset, j = 0; i < top.size(); ++i, ++j) {
        top[i]->ShareData(*this->recur_output_blobs_[j]);
        top[i]->ShareDiff(*this->recur_output_blobs_[j]);
      }
    }
  }

  // TODO(agethen): Clean up and simplification
  template <typename Dtype>
  void ConvLSTMLayer<Dtype>::LayerSetUp( const std::vector<Blob<Dtype>*>& bottom, const std::vector<Blob<Dtype>*>& top) {
    
    // Max. 1 for hidden, 1 for input
    CHECK_LE( this->layer_param_.lstm_conv_param_size(), 2 );
    CHECK_GE( this->layer_param_.lstm_conv_param_size(), 1 );

    if( this->layer_param_.lstm_conv_param_size() == 2 ){
      CHECK_EQ( this->layer_param_.lstm_conv_param( 0 ).num_output(),
                this->layer_param_.lstm_conv_param( 1 ).num_output() );

    }
    num_output_channels_ = this->layer_param_.lstm_conv_param( 0 ).num_output();

    in_shape_.clear();

    for( int i = 0; i < bottom[0]->num_axes(); ++i )
      in_shape_.push_back( bottom[0]->shape( i ) );

    RecurrentLayer<Dtype>::LayerSetUp( bottom, top );


    std::vector<std::string> recur_output_names;
    RecurrentOutputBlobNames( &recur_output_names );
    this->last_layer_index_ = this->last_layer_index_ - recur_output_names.size();
  }

  template <typename Dtype>
  void ConvLSTMLayer<Dtype>::RecurrentInputBlobNames( std::vector< std::string > * names ) const {
    names->resize(2);
    names->at(0) = "h_t=0";
    names->at(1) = "c_t=0";
  }

  template <typename Dtype>
  void ConvLSTMLayer<Dtype>::RecurrentOutputBlobNames( std::vector<std::string> * names ) const {
    names->resize(2);
    (*names)[0] = "h_t=T";
    (*names)[1] = "c_t=T";
  }

  // Shape of initial hidden and cell maps
  template <typename Dtype>
  void ConvLSTMLayer<Dtype>::RecurrentInputShapes( std::vector<BlobShape> * shapes ) const {
      
    const int num_blobs = 2;
    shapes->resize( num_blobs );

    for (int i = 0; i < num_blobs; ++i) {
      (*shapes)[i].Clear();

      // T = 1 for all hidden states
      (*shapes)[i].add_dim( 1 );

      // We assume that the feature dimension H x W is not reduced
      // Therefore, we only need to adjust the number of channels
      for( int j = 1; j < this->in_shape_.size(); j++ ){
        int is = this->in_shape_[j];

        if( j == 2 )
          is = num_output_channels_;

        // std::cout << "Shape @" << i << ":\t" << is << std::endl;

        (*shapes)[i].add_dim( is );
      }
    }
  }

  template <typename Dtype>
  void ConvLSTMLayer<Dtype>::OutputBlobNames( std::vector<std::string> * names ) const {
      int num_expose = this->layer_param_.lstm_debug_param().expose_size();
      names->resize( 1 + num_expose );

      (*names)[0] = "h";
      for( int i = 0; i < num_expose; ++i )
        (*names)[i+1] = this->layer_param_.lstm_debug_param().expose( i );
  }

  template<typename Dtype>
  void ConvLSTMLayer<Dtype>::FillUnrolledNet( NetParameter * net_param ) const {
    auto param = this->layer_param_.recurrent_param();

    auto debug_param = this->layer_param_.lstm_debug_param();

    std::cout << "Unrolling ConvLSTM" << std::endl;

    const std::vector<int> feature_shape = { this->in_shape_[2], this->in_shape_[3], this->in_shape_[4] };

    if( debug_param.ignore_x() ){
      // Discard all output to console from x
      // Not sure if this is actually necessary?
      LayerParameter * silence = net_param->add_layer();
      silence->set_type("Silence");
      silence->add_bottom( "x" );
    }

    if( debug_param.ignore_x() == false ){
      // The input shape is: T x N x C x H x W. Caffe Convolution can handle 5D Tensors by setting the `axis` param
      // Note this may cause trouble with fillers like xavier (but saves extra reshaping)

      auto lstm_conv_param = this->layer_param_.lstm_conv_param( 0 );
      if( lstm_conv_param.type() == "hidden" )
        lstm_conv_param = this->layer_param_.lstm_conv_param( 1 );

      lstm_conv_param.set_num_output( 4 * lstm_conv_param.num_output() );
      auto x_transform = ConvLSTMHelper<Dtype>::CreateConvLayer( net_param, &lstm_conv_param, "x->transform", "x", "x->transform", 2 );

      x_transform->add_param()->set_name( "x_transform" );
      x_transform->mutable_convolution_param()->set_bias_term( false );

      if( this->static_input_ ){
        // Note that static input is expected to be of shape: 1 x N x C_static x H x W
        auto static_transform  = ConvLSTMHelper<Dtype>::CreateConvLayer( net_param, &lstm_conv_param,  "static->input",   "x_static", "static->input",  2 );
        static_transform->mutable_convolution_param()->set_bias_term( false );
        static_transform->add_param()->set_name( "static_transform" );
      }
    } // ignore_x set?

    // Setup inital cell states c_0 and h_0 as output blobs.
    // In recurrent_layer.cpp, these blobs are connected to the corresponding input.
    std::vector< BlobShape > input_shapes;        
    RecurrentInputShapes( &input_shapes );

    std::vector< std::string > input_names{ "c_t=0", "h_t=0" };
    ConvLSTMHelper<Dtype>::CreateInputLayer( net_param, "input->cell_hidden", input_names, input_shapes );
    
    // We now slice the T x N x C x H x W tensor into 1 x N x C x H x W
    LayerParameter * x_slice_param;

    if( debug_param.ignore_x() == false )
      x_slice_param = ConvLSTMHelper<Dtype>::CreateSliceLayer( net_param, "W_xc_x_slice", "x->transform", 0 );

    // Slice cont marker from T x N into 1 x N
    auto cont_slice_param   = ConvLSTMHelper<Dtype>::CreateSliceLayer( net_param, "cont_slice", "cont", 0 );

    // Concatenation of h for final output
    // Do not use ConvLSTMHelper impl. here!
    LayerParameter output_concat_layer;
    output_concat_layer.set_name("h_concat");
    output_concat_layer.set_type("Concat");
    output_concat_layer.add_top("h");
    output_concat_layer.mutable_concat_param()->set_axis( 0 );

    // Enable gradient backpropagation to h_0, c_0 (in case we expose them for an encoder-decoder model)
    ConvLSTMHelper<Dtype>::CreateDummyForwardLayer( net_param, "dummy_forward_c0", "c_t=0", "c_t=0" );
    ConvLSTMHelper<Dtype>::CreateDummyForwardLayer( net_param, "dummy_forward_h0", "h_t=0", "h_t=0" );

    // Timesteps
    for( int t = 1; t <= this->T_; ++t ){
      std::cout << "Unrolling T=" << t << std::endl;

      std::string tm1s    = ConvLSTMHelper<Dtype>::int2str( t-1 );
      std::string ts      = ConvLSTMHelper<Dtype>::int2str( t );

      cont_slice_param->add_top( "cont_t=" + ts );

      if( debug_param.ignore_x() == false )
        x_slice_param->add_top( "x->transform->t=" + ts );           // Shape: 1 x N x 4C x H x W

      // Set h to zero if sequence marker 0.
      std::vector<std::string> scale_bottom{ "h_t=" + tm1s, "cont_t=" + ts };
      ConvLSTMHelper<Dtype>::CreateScaleLayer( net_param, "h_conted_t=" + tm1s, scale_bottom, "h_conted_t=" + tm1s );
      
      // Hidden state Convolutions
      auto lstm_conv_param = this->layer_param_.lstm_conv_param( 0 );
      if( lstm_conv_param.type() == "input" )
        lstm_conv_param = this->layer_param_.lstm_conv_param( 1 );

      // If no Hadamard Term, and input features are ignored, we directly generate the LSTM unit input "gate_input_" + ts (avoiding the sum layer)
      std::string hidden_top = (debug_param.disable_hadamard() && debug_param.ignore_x()) ? "gate_input_" + ts : "hidden->transform->" + tm1s;

      lstm_conv_param.set_num_output( 4 * lstm_conv_param.num_output() );
      auto hidden_transform  = ConvLSTMHelper<Dtype>::CreateConvLayer( net_param, &lstm_conv_param,  "hidden->transform->" + tm1s, "h_conted_t=" + tm1s, hidden_top,  2 );

      hidden_transform->add_param()->set_name(  "h->transform" );
      hidden_transform->add_param()->set_name(  "h->transform_bias" );
      
      if( !debug_param.disable_hadamard() ){
        auto hadamard_in = ConvLSTMHelper<Dtype>::CreateHadamard( 
            net_param, "hadamard->input_t=" + tm1s, "c_t=" + tm1s, "hadamard_in_t=" + ts, debug_param.axis_hadamard(), debug_param.num_axes_hadamard() );
        auto hadamard_fog = ConvLSTMHelper<Dtype>::CreateHadamard( 
            net_param, "hadamard->forget_t=" + tm1s, "c_t=" + tm1s, "hadamard_fog_t=" + ts, debug_param.axis_hadamard(), debug_param.num_axes_hadamard() );
        auto hadamard_out = ConvLSTMHelper<Dtype>::CreateHadamard( 
            net_param, "hadamard->output_t=" + tm1s, "c_t=" + tm1s, "hadamard_out_t=" + ts, debug_param.axis_hadamard(), debug_param.num_axes_hadamard() );
        // There is no hadamard term for the gate activations. For convenience, we just use DummyDataLayer which supplies us with zeros.
        ConvLSTMHelper<Dtype>::CreateDummyData( net_param, "hadamard_gat_t=" + ts, "hadamard_gat_t=" + ts, input_shapes[0] );
        
        hadamard_in->add_param()->set_name( "hadamard.input" );
        hadamard_fog->add_param()->set_name( "hadamard.forget" );
        hadamard_out->add_param()->set_name( "hadamard.output" );

        auto concat_hadamard = ConvLSTMHelper<Dtype>::CreateConcatLayer( net_param, "concat_hadamard_t=" + ts, "hadamard_t=" + ts, 2 );
        concat_hadamard->add_bottom( "hadamard_in_t=" + ts );
        concat_hadamard->add_bottom( "hadamard_fog_t=" + ts );
        concat_hadamard->add_bottom( "hadamard_out_t=" + ts );
        concat_hadamard->add_bottom( "hadamard_gat_t=" + ts );
      }

      // Recall that for i,f,c,o the preactivation component is the sum of input 'x_t' and hidden state 'h_{t-1}' + hadamard term
      // x --> W_xc_x --> W_xc_x_<timestep> --> gate_input_<timestep>
      std::vector<std::string> sum_bottom { "hidden->transform->" + tm1s };

      if( debug_param.ignore_x() == false )
        sum_bottom.push_back( "x->transform->t=" + ts );
      
      if( !debug_param.disable_hadamard() )
         sum_bottom.push_back( "hadamard_t=" + ts );

      if( this->static_input_ )
        sum_bottom.push_back( "static->input" );

      if( !debug_param.ignore_x() || !debug_param.disable_hadamard() )
        ConvLSTMHelper<Dtype>::CreateSumLayer( net_param, "gate_input_" + ts, sum_bottom, "gate_input_" + ts );

      std::cout << "Building ConvLSTMUnit layer" << std::endl;
      // Create a Conv LSTM Unit for this timestep, connecting the
      // previous timestep c_tm1s and the current output c_ts.
      // Note that ConvLSTMUnit and LSTMUnit only differ in accepted input shapes
      LayerParameter * lstm_unit_param = net_param->add_layer();
      lstm_unit_param->set_name( "unit_t=" + ts );
      lstm_unit_param->set_type( "ConvLSTMUnit" );
      lstm_unit_param->add_bottom( "c_t=" + tm1s );       // Previous cell-state
      lstm_unit_param->add_bottom( "gate_input_" + ts );  // Gate input
      lstm_unit_param->add_bottom( "cont_t=" + ts );      // Sequence indicator vector
      lstm_unit_param->add_top( "c_t=" + ts );
      lstm_unit_param->add_top( "h_t=" + ts );

      // This will create a very large blob....
      output_concat_layer.add_bottom( "h_t=" + ts );
    }

    // Note: This layer has to be built AFTER all lstm units are set-up!
    net_param->add_layer()->CopyFrom( output_concat_layer );

    // If we want to backpropagate over the hidden_state output, we need to add a dummy forward for h_t=T
    // This is because, unlike c_t=T, h_t=T is being used (in the concat layer), and therefore causes a Split layer to spawn
    // For consistency, we will also create a dummy_forward for c_t=T
    ConvLSTMHelper<Dtype>::CreateDummyForwardLayer( net_param, "dummy_forward_h", "h_t=" + ConvLSTMHelper<Dtype>::int2str(this->T_), "h_t=T" );
    ConvLSTMHelper<Dtype>::CreateDummyForwardLayer( net_param, "dummy_forward_c", "c_t=" + ConvLSTMHelper<Dtype>::int2str(this->T_), "c_t=T" );

    if( this->expose_hidden_ ){
      // We need to add extra pseudo-losses if we expose h_T, c_T
      std::vector<std::string> recur_output_names;
      RecurrentOutputBlobNames( &recur_output_names );

      for( int i = 0; i < recur_output_names.size(); ++i ){
        auto layer = ConvLSTMHelper<Dtype>::CreateReductionLayer( net_param, recur_output_names[i] + "_pseudoloss",
                                                                  recur_output_names[i], recur_output_names[i] + "_pseudoloss" );
        layer->add_loss_weight(1);
      }
    }

  }

  INSTANTIATE_CLASS(ConvLSTMLayer);
  REGISTER_LAYER_CLASS(ConvLSTM);

}  // namespace caffe
