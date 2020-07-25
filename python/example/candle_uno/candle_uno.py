from flexflow.core import *
import uno as benchmark
from default_utils import finalize_parameters
from uno_data import CombinedDataLoader

def initialize_parameters(default_model='uno_default_model.txt'):
  # Build benchmark object
  unoBmk = benchmark.BenchmarkUno(benchmark.file_path, default_model, 'keras',
                                  prog='uno_baseline', desc='Build neural network based models to predict tumor response to single and paired drugs.')
  # Initialize parameters
  gParameters = finalize_parameters(unoBmk)
  # benchmark.logger.info('Params: {}'.format(gParameters))
  return gParameters

class Struct:
  def __init__(self, **entries):
    self.__dict__.update(entries)

def build_model(loader, args, permanent_dropout=True, silent=False):
  input_models = {}
  dropout_rate = args.dropout
  for fea_type, shape in loader.feature_shapes.items():
    base_type = fea_type.split('.')[0]
    if base_type in ['cell', 'drug']:
      if args.dense_cell_feature_layers is not None and base_type == 'cell':
        dense_feature_layers = args.dense_cell_feature_layers
      elif args.dense_drug_feature_layers is not None and base_type == 'drug':
        dense_feature_layers = args.dense_drug_feature_layers
      else:
        dense_feature_layers = args.dense_feature_layers

      box = build_feature_model(input_shape=shape, name=fea_type,
                                dense_layers=dense_feature_layers,
                                dropout_rate=dropout_rate, permanent_dropout=permanent_dropout)
      if not silent:
        logger.debug('Feature encoding submodel for %s:', fea_type)
        box.summary(print_fn=logger.debug)
      input_models[fea_type] = box

  inputs = []
  encoded_inputs = []
  for fea_name, fea_type in loader.input_features.items():
    shape = loader.feature_shapes[fea_type]
    fea_input = Input(shape, name='input.' + fea_name)
    inputs.append(fea_input)
    if fea_type in input_models:
      input_model = input_models[fea_type]
      encoded = input_model(fea_input)
    else:
      encoded = fea_input
    encoded_inputs.append(encoded)

  merged = keras.layers.concatenate(encoded_inputs)

  h = merged
  for i, layer in enumerate(args.dense):
    x = h
    h = Dense(layer, activation=args.activation)(h)
    if dropout_rate > 0:
      if permanent_dropout:
        h = PermanentDropout(dropout_rate)(h)
      else:
        h = Dropout(dropout_rate)(h)
    if args.residual:
      try:
        h = keras.layers.add([h, x])
      except ValueError:
        pass
  output = Dense(1)(h)

  return Model(inputs, output)

def top_level_task():
  params = initialize_parameters()
  args = Struct(**params)
  ffconfig = FFConfig()
  ffconfig.parse_args()
  ffmodel = FFModel(ffconfig)
  loader = CombinedDataLoader(seed=args.rng_seed)
  print(loader)
  loader.load(cache=args.cache,
              ncols=args.feature_subsample,
              agg_dose=args.agg_dose,
              cell_features=args.cell_features,
              drug_features=args.drug_features,
              drug_median_response_min=args.drug_median_response_min,
              drug_median_response_max=args.drug_median_response_max,
              use_landmark_genes=args.use_landmark_genes,
              use_filtered_genes=args.use_filtered_genes,
              cell_feature_subset_path=args.cell_feature_subset_path or args.feature_subset_path,
              drug_feature_subset_path=args.drug_feature_subset_path or args.feature_subset_path,
              preprocess_rnaseq=args.preprocess_rnaseq,
              single=args.single,
              train_sources=args.train_sources,
              test_sources=args.test_sources,
              embed_feature_source=not args.no_feature_source,
              encode_response_source=not args.no_response_source,
              use_exported_data=args.use_exported_data,
              )

  target = args.agg_dose or 'Growth'
  val_split = args.val_split
  train_split = 1 - val_split

  if args.export_csv:
    fname = args.export_csv
    loader.partition_data(cv_folds=args.cv, train_split=train_split, val_split=val_split,
                          cell_types=args.cell_types, by_cell=args.by_cell, by_drug=args.by_drug,
                          cell_subset_path=args.cell_subset_path, drug_subset_path=args.drug_subset_path)
    train_gen = CombinedDataGenerator(loader, batch_size=args.batch_size, shuffle=args.shuffle)
    val_gen = CombinedDataGenerator(loader, partition='val', batch_size=args.batch_size, shuffle=args.shuffle)

    x_train_list, y_train = train_gen.get_slice(size=train_gen.size, dataframe=True, single=args.single)
    x_val_list, y_val = val_gen.get_slice(size=val_gen.size, dataframe=True, single=args.single)
    df_train = pd.concat([y_train] + x_train_list, axis=1)
    df_val = pd.concat([y_val] + x_val_list, axis=1)
    df = pd.concat([df_train, df_val]).reset_index(drop=True)
    if args.growth_bins > 1:
      df = uno_data.discretize(df, 'Growth', bins=args.growth_bins)
    df.to_csv(fname, sep='\t', index=False, float_format="%.3g")
    return

  if args.export_data:
    fname = args.export_data
    loader.partition_data(cv_folds=args.cv, train_split=train_split, val_split=val_split,
                          cell_types=args.cell_types, by_cell=args.by_cell, by_drug=args.by_drug,
                          cell_subset_path=args.cell_subset_path, drug_subset_path=args.drug_subset_path)
    train_gen = CombinedDataGenerator(loader, batch_size=args.batch_size, shuffle=args.shuffle)
    val_gen = CombinedDataGenerator(loader, partition='val', batch_size=args.batch_size, shuffle=args.shuffle)
    store = pd.HDFStore(fname, complevel=9, complib='blosc:snappy')

    config_min_itemsize = {'Sample': 30, 'Drug1': 10}
    if not args.single:
      config_min_itemsize['Drug2'] = 10

    for partition in ['train', 'val']:
      gen = train_gen if partition == 'train' else val_gen
      for i in range(gen.steps):
        x_list, y = gen.get_slice(size=args.batch_size, dataframe=True, single=args.single)
        for j, input_feature in enumerate(x_list):
          input_feature.columns = [''] * len(input_feature.columns)
          store.append('x_{}_{}'.format(partition, j), input_feature.astype('float32'), format='table', data_column=True)
        store.append('y_{}'.format(partition), y.astype({target: 'float32'}), format='table', data_column=True,
                     min_itemsize=config_min_itemsize)
        logger.info('Generating {} dataset. {} / {}'.format(partition, i, gen.steps))

    # save input_features and feature_shapes from loader
    store.put('model', pd.DataFrame())
    store.get_storer('model').attrs.input_features = loader.input_features
    store.get_storer('model').attrs.feature_shapes = loader.feature_shapes

    store.close()
    logger.info('Completed generating {}'.format(fname))
    return

  if args.use_exported_data is None:
    loader.partition_data(cv_folds=args.cv, train_split=train_split, val_split=val_split,
                          cell_types=args.cell_types, by_cell=args.by_cell, by_drug=args.by_drug,
                          cell_subset_path=args.cell_subset_path, drug_subset_path=args.drug_subset_path)

  model = build_model(loader, args)

if __name__ == "__main__":
  print("candle uno")
  top_level_task()
