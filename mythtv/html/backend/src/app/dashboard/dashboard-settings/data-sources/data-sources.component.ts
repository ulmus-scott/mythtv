import { AfterViewInit, Component, Input, OnInit, ViewChild } from '@angular/core';
import { NgForm, FormsModule } from '@angular/forms';
import { TranslateService, TranslateModule } from '@ngx-translate/core';
import { MythService } from 'src/app/services/myth.service';
import { ButtonModule } from 'primeng/button';
import { MessageModule } from 'primeng/message';
import { NgIf } from '@angular/common';
import { CheckboxModule } from 'primeng/checkbox';
import { SharedModule } from 'primeng/api';
import { CardModule } from 'primeng/card';
import { SelectModule } from 'primeng/select';
import { DashboardSettingsComponent } from '../dashboard-settings.component';

@Component({
    selector: 'app-data-sources',
    templateUrl: './data-sources.component.html',
    styleUrls: ['./data-sources.component.css'],
    standalone: true,
    imports: [FormsModule, CardModule, SharedModule, SelectModule, CheckboxModule, NgIf, MessageModule, ButtonModule, TranslateModule]
})
export class DataSourcesComponent implements OnInit, AfterViewInit {

    @ViewChild("datasources") currentForm!: NgForm;
    @Input() parent!: DashboardSettingsComponent;
    @Input() tabIndex!: number;


    movieList = [
        { Label: 'TheMovieDB.org V3 movie', Value: "metadata/Movie/tmdb3.py" },
    ];

    tvList = [
        { Label: 'TheMovieDB.org V3 television', Value: "metadata/Television/tmdb3tv.py" },
        { Label: 'TheTVDatabaseV4', Value: "metadata/Television/ttvdb4.py" },
        { Label: 'TVmaze.com', Value: "metadata/Television/tvmaze.py" },
    ];


    successCount = 0;
    errorCount = 0;

    MovieGrabber = "metadata/Movie/tmdb3.py";
    TelevisionGrabber = "metadata/Television/ttvdb4.py";
    DailyArtworkUpdates = false;

    constructor(private mythService: MythService, private translate: TranslateService) {
    }

    ngOnInit(): void {
        this.loadValues();
        this.markPristine();
        this.parent.children[this.tabIndex] = this;
    }

    dirty() {
        return this.currentForm.dirty;
    }

    ngAfterViewInit() {
    }

    loadValues() {
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "MovieGrabber", Default: "metadata/Movie/tmdb3.py" })
            .subscribe({
                next: data => this.MovieGrabber = data.String,
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "TelevisionGrabber", Default: "metadata/Television/ttvdb4.py" })
            .subscribe({
                next: data => this.TelevisionGrabber = data.String,
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "DailyArtworkUpdates", Default: "0" })
            .subscribe({
                next: data => this.DailyArtworkUpdates = (data.String == "1"),
                error: () => this.errorCount++
            });
    }

    markPristine() {
        setTimeout(() => {
            this.currentForm.form.markAsPristine();
            this.parent.showDirty();
        }, 100);
    }

    swObserver = {
        next: (x: any) => {
            if (x.bool)
                this.successCount++;
            else {
                this.errorCount++;
                if (this.currentForm)
                    this.currentForm.form.markAsDirty();
            }
        },
        error: (err: any) => {
            console.error(err);
            this.errorCount++;
            if (this.currentForm)
                this.currentForm.form.markAsDirty();
        },
    };

    saveForm() {
        this.successCount = 0;
        this.errorCount = 0;

        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "MovieGrabber",
            Value: String(this.MovieGrabber)
        }).subscribe(this.swObserver);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "TelevisionGrabber",
            Value: String(this.TelevisionGrabber)
        }).subscribe(this.swObserver);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "DailyArtworkUpdates",
            Value: this.DailyArtworkUpdates ? "1" : "0"
        }).subscribe(this.swObserver);
    }

}
