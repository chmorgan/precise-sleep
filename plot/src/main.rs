use plotly::{ImageFormat, Plot};
use plotly::Layout;
use plotly::layout::Axis;
use plotly::common::Title;

use plotly::box_plot::BoxPoints;
use plotly::BoxPlot;

use humanize_duration::Truncate;
use humanize_duration::prelude::DurationExt;

#[derive(Debug, Clone, serde::Deserialize, serde::Serialize)]
struct Record {
    sleep_type: String,
    target_ns: i64,
    actual_ns: i64
}

fn load_from_file(file_path: &str) -> Vec<Record>
{
    let file = std::fs::File::open(file_path).expect("file was not found");
    let mut reader = csv::Reader::from_reader(file);

    let records: Vec<Record> = reader
        .deserialize()
        .map(|record| {
                let r: Record = record.unwrap();
                r
            }
        )
        .collect();

    records
}

fn get_records<'a>(records: &Vec<Record>, sleep_type: &str, target_ns: i64) -> Vec<Record>
{
    let y= records.iter()
    .filter_map(|record|
    {
        if record.sleep_type == sleep_type &&
            record.target_ns == target_ns
        {
            let c = (*record).clone();
            Some(c)
        } else
        {
            None
        }
    })
    .collect();

    y
}

use clap::Parser;

#[derive(Parser)]
#[command(version, about, long_about = None)]
struct Cli
{
    #[arg(short, long, value_name = "results csv filename")]
    results: String,
}
fn main() {
    let cli = Cli::parse();

    let records = load_from_file(cli.results.as_str());

    // find the unique target durations
    let mut unique_durations: Vec<i64> = records.iter()
                    .map(|record| record.target_ns)
                    .collect();
    unique_durations.sort();
    unique_durations.dedup();

    // find the unique sleep_type values
    let mut sleep_types: Vec<String> = records.iter()
                    .map(|record| record.sleep_type.clone())
                    .collect();
    sleep_types.sort();
    sleep_types.dedup();

    println!("unique_durations {:#?}", unique_durations);
    println!("sleep_types {:#?}", sleep_types);

    for duration_ns in unique_durations
    {
        println!("duration_ns {}", duration_ns);

        let duration = std::time::Duration::from_nanos(duration_ns as u64);

        let human_duration = duration.human(Truncate::Nano);

        let mut plot = Plot::new();

        for sleep_type in &sleep_types
        {
            println!("\tsleep_type {}", sleep_type);

            let filtered_records = get_records(&records,
                    sleep_type.as_str(), duration_ns);

            let values = filtered_records.iter().map(|record| (record.actual_ns - record.target_ns) as f64 / 1e6).collect();

//            println!("values {:#?}", values);

            let trace1 = BoxPlot::new(values)
                .box_points(BoxPoints::All)
                .jitter(0.3)
                .point_pos(-1.8)
                .name(sleep_type);
            plot.add_trace(trace1);
        }

        let layout = Layout::new().title(Title::with_text(format!("Error for sleep time of {}", human_duration)))
            .y_axis(Axis::new().title(Title::with_text("Error in ms")));
        plot.set_layout(layout);

        plot.write_image(format!("duration_{}.png", human_duration),
                ImageFormat::PNG, 300, 800, 1.0);
    }
}

