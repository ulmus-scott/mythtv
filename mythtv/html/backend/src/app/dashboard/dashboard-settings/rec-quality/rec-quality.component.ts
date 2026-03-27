import { AfterViewInit, Component, Input, OnInit, ViewChild } from '@angular/core';
import { NgForm, FormsModule } from '@angular/forms';
import { GuideService } from 'src/app/services/guide.service';
import { MythService } from 'src/app/services/myth.service';
import { TranslateModule } from '@ngx-translate/core';
import { ButtonModule } from 'primeng/button';
import { MessageModule } from 'primeng/message';
import { NgIf } from '@angular/common';
import { FieldsetModule } from 'primeng/fieldset';
import { InputNumberModule } from 'primeng/inputnumber';
import { SharedModule } from 'primeng/api';
import { CardModule } from 'primeng/card';
import { Select, SelectModule } from "primeng/select";
import { DashboardSettingsComponent } from '../dashboard-settings.component';

@Component({
    selector: 'app-rec-quality',
    templateUrl: './rec-quality.component.html',
    styleUrls: ['./rec-quality.component.css'],
    imports: [FormsModule, CardModule, SharedModule, InputNumberModule, FieldsetModule, SelectModule, NgIf, MessageModule, ButtonModule, TranslateModule, Select]
})
export class RecQualityComponent implements OnInit, AfterViewInit {

    @ViewChild("quality") currentForm!: NgForm;
    @Input() parent!: DashboardSettingsComponent;
    @Input() tabIndex!: number;

    successCount = 0;
    errorCount = 0;
    CategoryList: string[] = [];

    RecordPreRoll = 0;
    RecordOverTime = 0;
    MaxStartGap = 15;
    MaxEndGap = 15;
    MinimumRecordingQuality = 95;
    OverTimeCategory = '';
    CategoryOverTime = 30;


    constructor(private mythService: MythService,
        private guideService: GuideService) {
    }

    ngOnInit(): void {
        this.loadValues();
        this.parent.children[this.tabIndex] = this;
    }

    ngAfterViewInit() {
    }

    dirty() {
        return this.currentForm.dirty;
    }

    loadValues() {
        this.guideService.GetCategoryList().subscribe((data) => {
            this.CategoryList = data.CategoryList;
            this.CategoryList.unshift('');
            this.markPristine();
        });

        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "RecordPreRoll", Default: "0" })
            .subscribe({
                next: data => this.RecordPreRoll = Number(data.String),
                error: () => this.errorCount++
            });

        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "RecordOverTime", Default: "0" })
            .subscribe({
                next: data => this.RecordOverTime = Number(data.String),
                error: () => this.errorCount++
            });

        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "MaxStartGap", Default: "15" })
            .subscribe({
                next: data => this.MaxStartGap = Number(data.String),
                error: () => this.errorCount++
            });

        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "MaxEndGap", Default: "15" })
            .subscribe({
                next: data => this.MaxEndGap = Number(data.String),
                error: () => this.errorCount++
            });

        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "MinimumRecordingQuality", Default: "95" })
            .subscribe({
                next: data => this.MinimumRecordingQuality = Number(data.String),
                error: () => this.errorCount++
            });

        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "OverTimeCategory", Default: "" })
            .subscribe({
                next: data => this.OverTimeCategory = data.String,
                error: () => this.errorCount++
            });

        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "CategoryOverTime", Default: "30" })
            .subscribe({
                next: data => this.CategoryOverTime = Number(data.String),
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
            HostName: '_GLOBAL_', Key: "RecordPreRoll",
            Value: String(this.RecordPreRoll)
        }).subscribe(this.swObserver);

        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "RecordOverTime",
            Value: String(this.RecordOverTime)
        }).subscribe(this.swObserver);

        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "MaxStartGap",
            Value: String(this.MaxStartGap)
        }).subscribe(this.swObserver);

        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "MaxEndGap",
            Value: String(this.MaxEndGap)
        }).subscribe(this.swObserver);

        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "MinimumRecordingQuality",
            Value: String(this.MinimumRecordingQuality)
        }).subscribe(this.swObserver);

        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "OverTimeCategory",
            Value: this.OverTimeCategory
        }).subscribe(this.swObserver);

        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "CategoryOverTime",
            Value: String(this.CategoryOverTime)
        }).subscribe(this.swObserver);

    }

}
